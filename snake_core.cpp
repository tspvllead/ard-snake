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
    addHead(4, 1);
    addHead(4, 2);
    addHead(4, 3);
    placeFood(num);
}

void Game::addHead(uint8_t row, uint8_t col) {
    head = static_cast<uint8_t>((head + 1) & (snake_max - 1));
    body[head].col = col;
    body[head].row = row;
    count++;
    setCell(row, col);
}

void Game::removeTail() {
    uint8_t tail =
        static_cast<uint8_t>((head + snake_max - count + 1) & (snake_max - 1));
    clearCell(body[tail].row, body[tail].col);
    count--;
}

void Game::requestDirection(Direction dir) {
    if (((static_cast<uint8_t>(current) + 2) & 3) ==
        static_cast<uint8_t>(dir)) {
        return;
    }
    pending = dir;
}

void Game::tick(uint8_t t) {
    if (snake_state != Status::Running) {
        return;
    }
    current = pending;
    Pos head_pos = body[head];
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
    if (!snake_ate) {
        removeTail();
    }
    if (testCell(head_pos.row, head_pos.col)) {
        snake_state = Status::Dead;
        return;
    }
    addHead(head_pos.row, head_pos.col);
    if (snake_ate) {
        if (count == snake_max) {
            snake_state = Status::Won;
            return;
        } else {
            placeFood(t);
        }
    }
}

void Game::placeFood(uint8_t rn) {
    uint8_t free_cells = snake_max - count;
    uint8_t rank = rn % free_cells;
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