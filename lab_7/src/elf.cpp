#include "elf.hpp"

#include "bear.hpp"
#include "robber.hpp"

namespace lab7 {

Elf::Elf(const std::string& name, int x, int y)
    : NPC(NpcType::Elf, name, x, y) {}

bool Elf::Accept(std::shared_ptr<FightVisitor> visitor) {
  return visitor->Visit(std::dynamic_pointer_cast<Elf>(shared_from_this()));
}

bool Elf::Visit(std::shared_ptr<Bear> /*defender*/) {
  return false;
}

bool Elf::Visit(std::shared_ptr<Elf> /*defender*/) {
  return false;
}

bool Elf::Visit(std::shared_ptr<Robber> defender) {
  if (!defender || !defender->IsAlive() || !IsAlive()) {
    return false;
  }
  int attack = RollDice();
  int defense = defender->RollDice();
  return attack > defense;
}

}  // namespace lab7
