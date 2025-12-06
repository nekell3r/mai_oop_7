#include "elf.hpp"
#include "bear.hpp"
#include "robber.hpp"

Elf::Elf(const std::string& name, int x, int y)
    : NPC(NpcType::Elf, name, x, y) {}

bool Elf::Accept(std::shared_ptr<FightVisitor> visitor) {
  return visitor->Visit(std::dynamic_pointer_cast<Elf>(shared_from_this()));
}

// Combat with dice: attack vs defense
bool Elf::Fight(std::shared_ptr<Bear> other) {
  if (!other || !other->IsAlive() || !IsAlive()) {
    return false;
  }
  int attack = RollDice();
  int defense = other->RollDice();
  return attack > defense;
}

bool Elf::Fight(std::shared_ptr<Elf> other) {
  if (!other || !other->IsAlive() || !IsAlive()) {
    return false;
  }
  int attack = RollDice();
  int defense = other->RollDice();
  return attack > defense;
}

bool Elf::Fight(std::shared_ptr<Robber> other) {
  if (!other || !other->IsAlive() || !IsAlive()) {
    return false;
  }
  int attack = RollDice();
  int defense = other->RollDice();
  return attack > defense;
}

// Visitor implementation for Elf
bool Elf::Visit(std::shared_ptr<Bear> defender) {
  return Fight(defender);
}

bool Elf::Visit(std::shared_ptr<Elf> defender) {
  return Fight(defender);
}

bool Elf::Visit(std::shared_ptr<Robber> defender) {
  return Fight(defender);
}
