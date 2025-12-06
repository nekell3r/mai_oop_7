#pragma once

#include "npc.hpp"
#include "fight_visitor.hpp"

class Elf : public NPC, public FightVisitor {
 public:
  Elf(const std::string& name, int x, int y);

  bool Accept(std::shared_ptr<FightVisitor> visitor) override;

  bool Fight(std::shared_ptr<Bear> other) override;
  bool Fight(std::shared_ptr<Elf> other) override;
  bool Fight(std::shared_ptr<Robber> other) override;

  // Visitor methods
  bool Visit(std::shared_ptr<Bear> defender) override;
  bool Visit(std::shared_ptr<Elf> defender) override;
  bool Visit(std::shared_ptr<Robber> defender) override;
};
