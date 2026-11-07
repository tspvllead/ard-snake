#include "snake_core.h"

void Game::setCell(uint8_t row, uint8_t col) {
    uint8_t mask = static_cast<uint8_t>(1 << col);
    bitmap[row] = static_cast<uint8_t>(mask | bitmap[row]);
}

void Game::clearCell(uint8_t row, uint8_t col) {
    uint8_t mask = static_cast<uint8_t>(1 << col);
    bitmap[row] = static_cast<uint8_t>(~mask & bitmap[row]);
}

bool Game::testCell(uint8_t row, uint8_t col) const {
    uint8_t mask = static_cast<uint8_t>(1 << col);
    return (mask & bitmap[row]) != 0;
}

Status Game::status() const { return snake_state; }

uint8_t Game::length() const { return count; }

void Game::frame(uint8_t grid[8]) const {
    for (uint8_t i = 0; i < board_size; i++) {
        grid[i] = bitmap[i];
    }
    uint8_t mask = static_cast<uint8_t>(1 << food.col);
    grid[food.row] = static_cast<uint8_t>(grid[food.row] | mask);
}

void Game::reset(uint8_t num) {
    head = 0;
    count = 0;
    for (uint8_t i = 0; i < board_size; i++) {
        bitmap[i] = 0;
    }
    current = Direction::Right;
    pending = Direction::Right;
    snake_state = Status::Running;
    food.row = 3;
    food.col = 5;
}

void Game::requestDirection(Direction dir) {}
void Game::tick(uint8_t t) {}