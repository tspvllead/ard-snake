#include "snake_core.h"

// Cell Helpers with same logic, all use bit shift to isolate needed column at
// given row. Then uses inversion, AND and OR to check or update the cells

// sets a cell to active
void Game::setCell(uint8_t row, uint8_t col) {
    uint8_t mask = static_cast<uint8_t>(1 << col);
    bitmap[row] = static_cast<uint8_t>(mask | bitmap[row]);
}

// sets a cell to empty
void Game::clearCell(uint8_t row, uint8_t col) {
    uint8_t mask = static_cast<uint8_t>(1 << col);
    bitmap[row] = static_cast<uint8_t>(~mask & bitmap[row]);
}

// checks a cell, returns true if active
bool Game::testCell(uint8_t row, uint8_t col) const {
    uint8_t mask = static_cast<uint8_t>(1 << col);
    return (mask & bitmap[row]) != 0;
}

Status Game::status() const { return snake_state; }

uint8_t Game::length() const { return count; }

// places board into a supplied array and puts food bit onto it
// food only placed in this array, board only has body
void Game::frame(uint8_t grid[board_size]) const {
    for (uint8_t i = 0; i < board_size; i++) {
        grid[i] = bitmap[i];
    }
    uint8_t mask = static_cast<uint8_t>(1 << food.col);
    grid[food.row] = static_cast<uint8_t>(grid[food.row] | mask);
}

// full reset of the board: empty cells, create 3-size snake, moving right
// place the food based off rnd, placeFood() is last because it reads bitmap
void Game::reset(uint8_t rnd) {
    head_index = 0;
    count = 0;
    for (uint8_t i = 0; i < board_size; i++) {
        bitmap[i] = 0;
    }
    current = Direction::Right;
    pending = Direction::Right;
    snake_state = Status::Running;
    addHead(4, 1);
    addHead(4, 2);
    addHead(4, 3);
    placeFood(rnd);
}

// creates head at the new position, updates body and board accordingly
void Game::addHead(uint8_t row, uint8_t col) {
    head_index = static_cast<uint8_t>((head_index + 1) & (snake_max - 1));
    body[head_index].col = col;
    body[head_index].row = row;
    count++; // count must be less than snake_max
    setCell(row, col);
}

// finds and removes last snake segment, using head position and snake length
// together with addHead moves the snake forward
void Game::removeTail() {
    // tail position formula fully explained in section 4 of DESIGN.md
    // the tail is 'count - 1' slots behind the head, uses current snake length,
    // so computed before decrement
    uint8_t tail_index = static_cast<uint8_t>(
        (head_index + snake_max - count + 1) & (snake_max - 1));
    clearCell(body[tail_index].row, body[tail_index].col);
    count--; // count must at least be 1
}

// updates pending(requested) direction, unless it's opposite of current
void Game::requestDirection(Direction dir) {
    if (((static_cast<uint8_t>(current) + 2) & 3) ==
        static_cast<uint8_t>(dir)) {
        return;
    }
    pending = dir;
}

// one tick of the game, no timers included
void Game::tick(uint8_t rnd) {
    if (snake_state != Status::Running) {
        return;
    }
    current = pending;
    // deliberate copy of head position to not overwrite it before addHead()
    Pos head_pos = body[head_index];
    // calculates new head position based on current direction
    // +7 is basically -1 due to wrapping logic(section 5 of DESIGN.md)
    switch (current) {
    case Direction::Up:
        head_pos.row =
            static_cast<uint8_t>((head_pos.row + 7) & (board_size - 1));
        break;
    case Direction::Left:
        head_pos.col =
            static_cast<uint8_t>((head_pos.col + 7) & (board_size - 1));
        break;
    case Direction::Down:
        head_pos.row =
            static_cast<uint8_t>((head_pos.row + 1) & (board_size - 1));
        break;
    case Direction::Right:
        head_pos.col =
            static_cast<uint8_t>((head_pos.col + 1) & (board_size - 1));
        break;
    }
    bool snake_ate = (head_pos.row == food.row) && (head_pos.col == food.col);
    // we don't remove tail if snake ate, which makes it grow
    if (!snake_ate) {
        removeTail();
    }
    // tail is removed if snake didn't eat, head can enter the former tail cell
    // food is never in the bitmap, so no special handling for eating needed
    if (testCell(head_pos.row, head_pos.col)) {
        snake_state = Status::Dead;
        // tail is removed so length is 1 lower than expected
        return;
    }
    addHead(head_pos.row, head_pos.col);
    if (snake_ate) {
        if (count == snake_max) {
            snake_state = Status::Won;
            return;
        }
        placeFood(rnd);
    }
}

// uses random supplied byte to find Nth empty cell on the board
// then updates food position
void Game::placeFood(uint8_t rnd) {
    // count must be less than snake_max
    uint8_t free_cells = snake_max - count;
    uint8_t rank = rnd % free_cells;
    for (uint8_t i = 0; i < board_size; i++) {
        for (uint8_t j = 0; j < board_size; j++) {
            if (!testCell(i, j)) {
                if (rank == 0) {
                    food.row = i;
                    food.col = j;
                    return;
                } else {
                    rank--;
                }
            }
        }
    }
}