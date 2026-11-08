#pragma once

// core logic of snake
// no i/o, no clock, no random - random bytes must be supplied by caller
// made to be ported over to arduino uno later
// detailed explanation of logic is in DESIGN.md

#include <stdint.h>

// size of play board, must be a power of 2, as cell wrap is done via masking
constexpr uint8_t board_size = 8;
// max size of snake, also must be a power of 2, equals board size squared
constexpr uint8_t snake_max = 64;

static_assert(board_size == 8, "board size is not meant to be changed");
static_assert(snake_max == 64,
              "max length of snake is not meant to be changed");

// opposite directions are 2 positions apart, as opposite check depends on it
enum class Direction : uint8_t { Up, Left, Down, Right };

// snake is dead only after biting itself, as walls wrap around
enum class Status : uint8_t { Running, Dead, Won };

struct Pos {
    uint8_t row, col; // both values are between 0 and board_size - 1(7)
};

class Game {
  private:
    // body[] and bitmap always describe the same snake, both are updated when
    // either addHead() or removeTail() is called
    Pos body[snake_max];
    // head_index - index of latest added position to the body
    // on the tick the snake dies, count is one less than it was before that
    // tick, since removeTail() already ran
    uint8_t head_index, count;
    // playing board where each bit represents one position on the board,
    // "ON" bit means a cell is body and only snake body is stored here,
    // food will be added later on a copy of bitmap to avoid confusion
    // each byte is a row and lowest bit is column 0
    uint8_t bitmap[board_size];
    Pos food;
    Direction current, pending;
    Status snake_state;
    // none of following helpers check boundaries, so row and col must always be
    // in between 0 and board_size - 1
    bool testCell(uint8_t row, uint8_t col) const;
    void setCell(uint8_t row, uint8_t col);
    void clearCell(uint8_t row, uint8_t col);
    void addHead(uint8_t row, uint8_t col);
    void removeTail();
    void placeFood(uint8_t rnd);

  public:
    // board initialization, called before anything, creates a 3-cell long
    // snake, status to running and places food based on rnd
    void reset(uint8_t rnd);
    // updates pending if input is legal(not opposite of current)
    void requestDirection(Direction dir);
    // moves snake if game is running, rnd is random byte for food placement
    void tick(uint8_t rnd);
    // creates a copy of body bitmap in supplied array(size of board_size)
    // places food on the board based on Pos food
    void frame(uint8_t grid[board_size]) const;
    Status status() const;
    uint8_t length() const;
};
