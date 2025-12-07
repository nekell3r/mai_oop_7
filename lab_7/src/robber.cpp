#include "robber.hpp"
#include "bear.hpp"
#include "elf.hpp"

Robber::Robber(const std::string& name, int x, int y)
    : NPC(NpcType::Robber, name, x, y) {}

bool Robber::Accept(std::shared_ptr<FightVisitor> visitor) {
  return visitor->Visit(std::dynamic_pointer_cast<Robber>(shared_from_this()));
}

// Visitor implementation for Robber - contains fight logic
// Robber kills Robbers (with dice roll)
bool Robber::Visit(std::shared_ptr<Bear> /*defender*/) {
  // Robber cannot kill Bear
  return false;
}

bool Robber::Visit(std::shared_ptr<Elf> /*defender*/) {
  // Robber cannot kill Elf
  return false;
}

bool Robber::Visit(std::shared_ptr<Robber> defender) {
  if (!defender || !defender->IsAlive() || !IsAlive()) {
    return false;
  }
  // Robber kills Robber - use dice roll
  int attack = RollDice();
  int defense = defender->RollDice();
  return attack > defense;
}
