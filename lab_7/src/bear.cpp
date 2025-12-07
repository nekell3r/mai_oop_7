#include "bear.hpp"
#include "elf.hpp"
#include "robber.hpp"

Bear::Bear(const std::string& name, int x, int y)
    : NPC(NpcType::Bear, name, x, y) {}

bool Bear::Accept(std::shared_ptr<FightVisitor> visitor) {
  return visitor->Visit(std::dynamic_pointer_cast<Bear>(shared_from_this()));
}

// Visitor implementation for Bear - contains fight logic
// Bear kills Elves (with dice roll)
bool Bear::Visit(std::shared_ptr<Bear> /*defender*/) {
  // Bear cannot kill Bear
  return false;
}

bool Bear::Visit(std::shared_ptr<Elf> defender) {
  if (!defender || !defender->IsAlive() || !IsAlive()) {
    return false;
  }
  // Bear kills Elf - use dice roll
  int attack = RollDice();
  int defense = defender->RollDice();
  return attack > defense;
}

bool Bear::Visit(std::shared_ptr<Robber> /*defender*/) {
  // Bear cannot kill Robber
  return false;
}
