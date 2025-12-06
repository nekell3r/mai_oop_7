#include "bear.hpp"
#include "elf.hpp"
#include "robber.hpp"

Bear::Bear(const std::string& name, int x, int y)
    : NPC(NpcType::Bear, name, x, y) {}

bool Bear::Accept(std::shared_ptr<FightVisitor> visitor) {
  return visitor->Visit(std::dynamic_pointer_cast<Bear>(shared_from_this()));
}

// Combat with dice: attack vs defense
bool Bear::Fight(std::shared_ptr<Bear> other) {
  if (!other || !other->IsAlive() || !IsAlive()) {
    return false;
  }
  int attack = RollDice();
  int defense = other->RollDice();
  return attack > defense;
}

bool Bear::Fight(std::shared_ptr<Elf> other) {
  if (!other || !other->IsAlive() || !IsAlive()) {
    return false;
  }
  int attack = RollDice();
  int defense = other->RollDice();
  return attack > defense;
}

bool Bear::Fight(std::shared_ptr<Robber> other) {
  if (!other || !other->IsAlive() || !IsAlive()) {
    return false;
  }
  int attack = RollDice();
  int defense = other->RollDice();
  return attack > defense;
}

// Visitor implementation for Bear
bool Bear::Visit(std::shared_ptr<Bear> defender) {
  return Fight(defender);
}

bool Bear::Visit(std::shared_ptr<Elf> defender) {
  return Fight(defender);
}

bool Bear::Visit(std::shared_ptr<Robber> defender) {
  return Fight(defender);
}
