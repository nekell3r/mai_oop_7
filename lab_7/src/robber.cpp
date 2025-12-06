#include "robber.hpp"
#include "bear.hpp"
#include "elf.hpp"

Robber::Robber(const std::string& name, int x, int y)
    : NPC(NpcType::Robber, name, x, y) {}

bool Robber::Accept(std::shared_ptr<FightVisitor> visitor) {
  return visitor->Visit(std::dynamic_pointer_cast<Robber>(shared_from_this()));
}

// Combat with dice: attack vs defense
bool Robber::Fight(std::shared_ptr<Bear> other) {
  if (!other || !other->IsAlive() || !IsAlive()) {
    return false;
  }
  int attack = RollDice();
  int defense = other->RollDice();
  return attack > defense;
}

bool Robber::Fight(std::shared_ptr<Elf> other) {
  if (!other || !other->IsAlive() || !IsAlive()) {
    return false;
  }
  int attack = RollDice();
  int defense = other->RollDice();
  return attack > defense;
}

bool Robber::Fight(std::shared_ptr<Robber> other) {
  if (!other || !other->IsAlive() || !IsAlive()) {
    return false;
  }
  int attack = RollDice();
  int defense = other->RollDice();
  return attack > defense;
}

// Visitor implementation for Robber
bool Robber::Visit(std::shared_ptr<Bear> defender) {
  return Fight(defender);
}

bool Robber::Visit(std::shared_ptr<Elf> defender) {
  return Fight(defender);
}

bool Robber::Visit(std::shared_ptr<Robber> defender) {
  return Fight(defender);
}
