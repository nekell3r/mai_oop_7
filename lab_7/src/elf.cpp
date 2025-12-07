#include "elf.hpp"
#include "bear.hpp"
#include "robber.hpp"

Elf::Elf(const std::string& name, int x, int y)
    : NPC(NpcType::Elf, name, x, y) {}

bool Elf::Accept(std::shared_ptr<FightVisitor> visitor) {
  return visitor->Visit(std::dynamic_pointer_cast<Elf>(shared_from_this()));
}

// Visitor implementation for Elf - contains fight logic
// Elf kills Robbers (with dice roll)
bool Elf::Visit(std::shared_ptr<Bear> /*defender*/) {
  // Elf cannot kill Bear
  return false;
}

bool Elf::Visit(std::shared_ptr<Elf> /*defender*/) {
  // Elf cannot kill Elf
  return false;
}

bool Elf::Visit(std::shared_ptr<Robber> defender) {
  if (!defender || !defender->IsAlive() || !IsAlive()) {
    return false;
  }
  // Elf kills Robber - use dice roll
  int attack = RollDice();
  int defense = defender->RollDice();
  return attack > defense;
}
