#include "robber.hpp"

#include "bear.hpp"
#include "elf.hpp"

namespace lab7 {

Robber::Robber(const std::string& name, int x, int y)
    : NPC(NpcType::Robber, name, x, y) {}

bool Robber::Accept(std::shared_ptr<FightVisitor> visitor) {
  return visitor->Visit(std::dynamic_pointer_cast<Robber>(shared_from_this()));
}

bool Robber::Visit(std::shared_ptr<Bear> /*defender*/) {
  return false;
}

bool Robber::Visit(std::shared_ptr<Elf> /*defender*/) {
  return false;
}

bool Robber::Visit(std::shared_ptr<Robber> defender) {
  if (!defender || !defender->IsAlive() || !IsAlive()) {
    return false;
  }
  int attack = RollDice();
  int defense = defender->RollDice();
  return attack > defense;
}

}  // namespace lab7
