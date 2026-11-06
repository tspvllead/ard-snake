#pragma once

#include <stdint.h>

constexpr uint8_t board_size = 8;
constexpr uint8_t snake_max = 64;

enum class Direction : uint8_t {Up, Left, Down, Right};

enum class Status : uint8_t {Running, Dead, Won};

struct Pos
{
    uint8_t row, col;
};

class Game
{
private:
    Pos body[snake_max];
    uint8_t head, count;
    uint8_t bitmap[board_size];
    Pos food;
    Direction current, pending;
    Status st;
public:
    void reset(uint8_t num);
    void requestDirection(Direction dir);
    void tick(uint8_t t);
    void frame(uint8_t grid[8]) const ;
    Status status() const ;
    uint8_t length() const ;
}; 
