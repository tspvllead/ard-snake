# Snake core: design notes

Background for `snake_core.h` / `snake_core.cpp`. The code comments stay short and point here for the reasoning.

## 1. Purpose and layers

The core is pure game logic. It does no I/O, reads no clock and owns no random source. This keeps it portable to an Arduino and testable on a PC.

```
input  ->  Game core  ->  display
(keys,     (state,        (terminal now,
 buttons)   rules)         LED matrix later)
```

- **Input** reports the player's latest direction request.
- **Core** advances one step per `tick` and exposes the board as 8 bytes.
- **Display** turns those 8 bytes into pixels.

Only input and display change when moving from the Windows terminal to the Arduino. The core stays as is.

## 2. Board and bit convention

The board is 8x8 with wrap-around: walls never kill, the snake re-enters on the opposite edge. The only way to die is to bite yourself.

`frame()` returns 8 bytes, one per row.

- Byte index = row. Row 0 is printed at the top.
- Bit position = column. **Column 0 is the lowest (least significant) bit.**
- A set bit means "lit".

Example: a row with columns 1, 2, 3 lit:

```
column:  7 6 5 4 3 2 1 0
bit:     0 0 0 0 1 1 1 0
```

If a display driver expects the opposite order, flip it in the display layer, never in the core.

### Bit operations used

Build a mask by shifting 1 left by the column, so only that column's bit is on. Then:

| Goal  | Operation on the row byte      |
|-------|--------------------------------|
| Set   | OR with the mask               |
| Clear | AND with the inverted mask     |
| Test  | AND with the mask, nonzero = on|

Use the single-character operators (`|`, `&`) for bits. The doubled ones (`||`, `&&`) are logical and do not work on individual bits. Intermediate results are promoted to `int`, so cast back to `uint8_t` when storing.

## 3. State and the central invariant

| Field | Meaning |
|-------|---------|
| `body[64]` | Ring buffer of positions, in the order the head visited them |
| `head_index` | Index of the slot holding the newest position |
| `count` | Snake length |
| `bitmap[8]` | The same snake as a picture, for instant "is this cell occupied?" |
| `food` | One position, never inside the body, never stored in `bitmap` |
| `current`, `pending` | Heading now, and the player's latest legal request |
| `snake_state` | Running, Dead or Won |

**Invariant: `body` and `bitmap` always describe the same snake.** Only `addHead` and `removeTail` change them after `reset`, and each updates both together. Updating one without the other is the classic bug here.

## 4. The ring buffer

`body` is a circular buffer of 64 slots: after slot 63 comes slot 0 again. Elements are never shifted or copied between slots. An index moves instead.

- The snake is the `count` slots ending at `head_index`. Older slots hold stale positions that nothing reads.
- **Moving:** remove the tail (`count` goes down, its bit is cleared), then add a new head (`head_index` steps forward, position stored, `count` up, bit set).
- **Growing:** same, but skip the tail removal.
- The buffer stores only positions in time order. Turns are not recorded. The tail retraces the head's path, so it turns where the head turned.

Wrapping uses a mask, which works because 64 is a power of two: `(index + 1) & 63`.

**Tail index:** the tail is `count - 1` slots behind `head_index`.

```
tail_index = (head_index + snake_max - count + 1) & (snake_max - 1)
```

- `+ snake_max` keeps unsigned arithmetic from dipping below zero.
- `+ 1` is the "minus one" in `count - 1`.
- Compute it **before** decreasing `count`, because the formula uses the current length.
- Sanity check: with `count` = 1, `tail_index` must equal `head_index`.

Example, with 8 slots instead of 64: `head_index` = 3, `count` = 3 gives slots 1, 2, 3, with the tail at slot 1. With `head_index` = 0 and `count` = 3, the tail index is `(0 + 8 - 3 + 1) & 7 = 6`, so the snake occupies slots 6, 7, 0.

## 5. One tick, in order

1. If the game is not running, return.
2. Copy `pending` into `current`.
3. Compute the new head position, with wrap-around.
4. Decide whether the move eats (new position equals the food).
5. If not eating, remove the tail.
6. If the new cell is occupied, set Dead and return.
7. Add the new head.
8. If eating: set Won if the snake now fills the board, otherwise place new food.

**Why step 5 comes before step 6:** the head may legally enter the cell the tail is just leaving. The food is never inside the body, so an eating move can never collide. Death can therefore only follow a non-eating move, where the tail has already been removed.

**Wrap arithmetic.** All coordinates are 0 to 7 and wrapped by masking with 7. Moving "minus one" is done as "plus 7", which avoids going below zero on an unsigned type: row 0 plus 7 is 7 (top to bottom), and row 4 plus 7 is 11, masked to 3.

| Direction | Coordinate | Step |
|-----------|-----------|------|
| Up        | row       | +7   |
| Down      | row       | +1   |
| Left      | col       | +7   |
| Right     | col       | +1   |

## 6. Direction requests

A request is ignored if it is the exact opposite of the **current** heading. It is compared against `current`, not `pending`. Otherwise two quick key presses between ticks could smuggle in a reversal (for example Up then Left while heading Right).

The enum order is Up, Left, Down, Right, so the opposite of a direction is two steps along, wrapping at four: `(d + 2) & 3`. Reordering the enum is fine as long as opposites stay two apart.

## 7. Food placement

Count the free cells (`snake_max - count`), reduce the random byte to a rank in `0 .. free - 1` with `%`, then walk the board row by row and stop at the free cell with that rank.

- It always terminates, even on a nearly full board.
- The random byte comes from the caller, so tests are deterministic.
- It must only be called when at least one cell is free (the win check guards this).
- It runs after `addHead`, so the new food can never land on the snake.
- This relies on `snake_max` equalling the number of cells (8 x 8 = 64).

Reference results, with the starting snake in row 4 columns 1 to 3 (61 free cells):

| `reset(n)` | Food at |
|------------|---------|
| 0  | (0,0) |
| 29 | (3,5) |
| 33 | (4,4) |
| 60 | (7,7) |
| 61 | (0,0) |

## 8. Known quirks

- **Death tick:** on a non-eating move the tail is removed before the collision test, so when the snake dies its picture and `length()` are one cell short. Harmless, since the game is over, but note it if `length()` is used as a final score.
- **Helpers do not check bounds.** Rows and columns must be 0 to 7. The wrap masking guarantees this inside the core.
- `reset` must be called before anything else, because the fields start uninitialized.
- The **Won** state has not been tested yet (it needs 64 cells filled).

## 9. Hand-verified scenarios

These were checked by hand against printed frames. They are good candidates for an automated test file.

- **Slide and wrap:** from `reset(0)`, repeated ticks with no input move the snake right and wrap from column 7 to column 0.
- **Reversal ignored:** heading Right, a Left request changes nothing. Heading Up, a Down request changes nothing.
- **Eating:** `reset(33)`, then `tick(0)`. The snake grows to 4 and the new food appears at (0,0).
- **Tail chase (survives):** `reset(29)`, two ticks, request Up and `tick(0)` (eats at (3,5), length 4, food moves to (0,0)), request Left and tick, request Down and tick. The snake forms a 2x2 square and keeps running.
- **Death:** `reset(29)`, two ticks, request Up and `tick(21)` (eats, new food at (2,5)), `tick(0)` (eats again, length 5), then Right, Down, Left with a tick each. The last move enters the body and the status becomes Dead.
- After death, further ticks change nothing.

## 10. Porting to Arduino

- The core uses only fixed-size arrays, `enum class`, `constexpr` and `static_assert`: no dynamic memory, no exceptions, no standard library containers.
- Supply the random byte from the Arduino's own source.
- Drive `tick` from the board's millisecond timer.
- Map buttons or a joystick to `requestDirection`.
- Send the 8 `frame()` bytes to the matrix driver, flipping bit or row order there if the wiring needs it.