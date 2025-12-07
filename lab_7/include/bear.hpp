#pragma once

#include "npc.hpp"
#include "fight_visitor.hpp"

class Bear : public NPC, public FightVisitor {
 public:
  Bear(const std::string& name, int x, int y);

  bool Accept(std::shared_ptr<FightVisitor> visitor) override;

  // Visitor methods - contain fight logic
  bool Visit(std::shared_ptr<Bear> /*defender*/) override;
  bool Visit(std::shared_ptr<Elf> /*defender*/) override;
  bool Visit(std::shared_ptr<Robber> /*defender*/) override;
};
