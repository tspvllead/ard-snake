#include "snake_core.h"
#include <iostream>

void print(uint8_t field[8]) {
    for (uint8_t i = 0; i < board_size; i++) {
        for (uint8_t j = 0; j < board_size; j++) {
            if (((field[i] >> j) & 1) == 1) {
                std::cout << "#";
            } else {
                std::cout << ".";
            }
        }
        std::cout << std::endl;
    }
}

int main() {
    Game play;
    play.reset(29);
    uint8_t field[8];
    play.frame(field);
    print(field);
    std::cout << std::endl;
    play.tick(0);
    play.frame(field);
    print(field);
    std::cout << std::endl;
    play.tick(0);
    play.frame(field);
    print(field);
    std::cout << std::endl;
    play.requestDirection(Direction::Up);
    play.tick(21);
    play.frame(field);
    print(field);
    std::cout << std::endl;
    play.tick(0);
    play.frame(field);
    print(field);
    play.requestDirection(Direction::Right);
    play.tick(0);
    play.frame(field);
    print(field);
    std::cout << std::endl;
    play.requestDirection(Direction::Down);
    play.tick(0);
    play.frame(field);
    print(field);
    std::cout << std::endl;
    play.requestDirection(Direction::Left);
    play.tick(0);
    play.frame(field);
    print(field);
    if (play.status() == Status::Dead) {
        std::cout << std::endl;
        std::cout << "gameover";
    }

    return 0;
}