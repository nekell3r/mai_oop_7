#pragma once

#include "fight_visitor.hpp"
#include "npc.hpp"

namespace lab7 {

class Robber : public NPC, public FightVisitor {
 public:
  Robber(const std::string& name, int x, int y);

  bool Accept(std::shared_ptr<FightVisitor> visitor) override;

  bool Visit(std::shared_ptr<Bear> /*defender*/) override;
  bool Visit(std::shared_ptr<Elf> /*defender*/) override;
  bool Visit(std::shared_ptr<Robber> /*defender*/) override;
};

}  // namespace lab7
