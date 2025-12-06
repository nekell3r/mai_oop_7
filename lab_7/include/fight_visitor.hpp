#pragma once

#include <memory>

class Bear;
class Elf;
class Robber;

class FightVisitor {
 public:
  virtual ~FightVisitor() = default;

  virtual bool Visit(std::shared_ptr<Bear> defender) = 0;
  virtual bool Visit(std::shared_ptr<Elf> defender) = 0;
  virtual bool Visit(std::shared_ptr<Robber> defender) = 0;
};
