#include "game.hpp"

#include <iostream>

int main() {
  Game game;
  
  std::cout << "Initializing game with 50 NPCs..." << std::endl;
  game.Initialize(50);
  
  std::cout << "Starting game (30 seconds)..." << std::endl;
  game.Run();
  
  return 0;
}

