#include "bear.hpp"

#include "elf.hpp"
#include "robber.hpp"

namespace lab7 {

Bear::Bear(const std::string& name, int x, int y)
    : NPC(NpcType::Bear, name, x, y) {}

bool Bear::Accept(std::shared_ptr<FightVisitor> visitor) {
  return visitor->Visit(std::dynamic_pointer_cast<Bear>(shared_from_this()));
}

bool Bear::Visit(std::shared_ptr<Bear> /*defender*/) {
  return false;
}

bool Bear::Visit(std::shared_ptr<Elf> defender) {
  if (!defender || !defender->IsAlive() || !IsAlive()) {
    return false;
  }
  int attack = RollDice();
  int defense = defender->RollDice();
  return attack > defense;
}

bool Bear::Visit(std::shared_ptr<Robber> /*defender*/) {
  return false;
}

}  // namespace lab7
