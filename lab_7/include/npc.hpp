#pragma once

#include <memory>
#include <ostream>
#include <shared_mutex>
#include <string>
#include <vector>

namespace lab7 {

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

  void Move(int new_x, int new_y);
  int GetMoveDistance() const;
  int GetKillDistance() const;

  virtual bool Accept(std::shared_ptr<FightVisitor> visitor) = 0;

  virtual void Print(std::ostream& os) const;
  virtual void Save(std::ostream& os) const;

  std::string GetName() const;
  int GetX() const;
  int GetY() const;
  NpcType GetType() const;

  int RollDice() const;

  friend std::ostream& operator<<(std::ostream& os, const NPC& npc);
};

}  // namespace lab7
