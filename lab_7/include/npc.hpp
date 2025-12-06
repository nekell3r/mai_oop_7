#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <cmath>
#include <mutex>
#include <shared_mutex>

// Forward declarations
class Bear;
class Elf;
class Robber;
class FightVisitor;
class IFightObserver;

enum class NpcType {
  Unknown = 0,
  Bear = 1,
  Elf = 2,
  Robber = 3
};

class NPC : public std::enable_shared_from_this<NPC> {
 protected:
  std::string name_;
  int x_;
  int y_;
  NpcType type_;
  bool alive_;
  mutable std::shared_mutex mutex_;
  std::vector<std::shared_ptr<IFightObserver>> observers_;

 public:
  NPC(NpcType type, const std::string& name, int x, int y);
  virtual ~NPC() = default;

  void Subscribe(std::shared_ptr<IFightObserver> observer);
  void FightNotify(const std::shared_ptr<NPC>& attacker, 
                   const std::shared_ptr<NPC>& defender, 
                   bool win);

  bool IsClose(const std::shared_ptr<NPC>& other, size_t distance) const;
  bool IsAlive() const;
  void Kill();

  // Movement
  void Move(int new_x, int new_y);
  int GetMoveDistance() const;
  int GetKillDistance() const;

  virtual bool Accept(std::shared_ptr<FightVisitor> visitor) = 0;

  virtual bool Fight(std::shared_ptr<Bear> other) = 0;
  virtual bool Fight(std::shared_ptr<Elf> other) = 0;
  virtual bool Fight(std::shared_ptr<Robber> other) = 0;

  virtual void Print(std::ostream& os) const;
  virtual void Save(std::ostream& os) const;

  // Getters with thread safety
  std::string GetName() const;
  int GetX() const;
  int GetY() const;
  NpcType GetType() const;

  // Roll 6-sided die for attack/defense
  int RollDice() const;

  friend std::ostream& operator<<(std::ostream& os, const NPC& npc);
};
