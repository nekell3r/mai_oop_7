#include "npc.hpp"
#include "observer.hpp"
#include "npc_types.hpp"

#include <random>

namespace {
  int RollD6() {
    thread_local std::random_device rd;
    thread_local std::mt19937 gen(rd());
    thread_local std::uniform_int_distribution<> dice(1, 6);
    return dice(gen);
  }
}

NPC::NPC(NpcType type, const std::string& name, int x, int y)
    : name_(name), x_(x), y_(y), type_(type), alive_(true) {
  // Ensure coordinates are within map bounds (0-100)
  if (x_ < 0) x_ = 0;
  if (x_ > 100) x_ = 100;
  if (y_ < 0) y_ = 0;
  if (y_ > 100) y_ = 100;
}

void NPC::Subscribe(std::shared_ptr<IFightObserver> observer) {
  std::unique_lock<std::shared_mutex> lock(mutex_);
  observers_.push_back(observer);
}

void NPC::FightNotify(const std::shared_ptr<NPC>& attacker,
                      const std::shared_ptr<NPC>& defender,
                      bool win) {
  // Copy observers to avoid holding lock during callback
  std::vector<std::shared_ptr<IFightObserver>> observers_copy;
  {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    observers_copy = observers_;
  }
  
  // Call observers without holding lock
  for (const auto& observer : observers_copy) {
    observer->OnFight(attacker, defender, win);
  }
}

bool NPC::IsClose(const std::shared_ptr<NPC>& other, size_t distance) const {
  // Lock in address order to prevent deadlock
  if (this < other.get()) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    std::shared_lock<std::shared_mutex> other_lock(other->mutex_);
    
    auto dx = x_ - other->x_;
    auto dy = y_ - other->y_;
    return (dx * dx + dy * dy) <= static_cast<int>(distance * distance);
  } else {
    std::shared_lock<std::shared_mutex> other_lock(other->mutex_);
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    auto dx = x_ - other->x_;
    auto dy = y_ - other->y_;
    return (dx * dx + dy * dy) <= static_cast<int>(distance * distance);
  }
}

bool NPC::IsAlive() const {
  std::shared_lock<std::shared_mutex> lock(mutex_);
  return alive_;
}

void NPC::Kill() {
  std::unique_lock<std::shared_mutex> lock(mutex_);
  alive_ = false;
}

void NPC::Move(int new_x, int new_y) {
  std::unique_lock<std::shared_mutex> lock(mutex_);
  if (!alive_) return;  // Dead NPCs don't move
  
  // Keep within map bounds
  if (new_x < 0) new_x = 0;
  if (new_x > 100) new_x = 100;
  if (new_y < 0) new_y = 0;
  if (new_y > 100) new_y = 100;
  
  x_ = new_x;
  y_ = new_y;
}

int NPC::GetMoveDistance() const {
  return NpcStats::GetMoveDistance(type_);
}

int NPC::GetKillDistance() const {
  return NpcStats::GetKillDistance(type_);
}

std::string NPC::GetName() const {
  std::shared_lock<std::shared_mutex> lock(mutex_);
  return name_;
}

int NPC::GetX() const {
  std::shared_lock<std::shared_mutex> lock(mutex_);
  return x_;
}

int NPC::GetY() const {
  std::shared_lock<std::shared_mutex> lock(mutex_);
  return y_;
}

NpcType NPC::GetType() const {
  return type_;
}

int NPC::RollDice() const {
  return RollD6();
}

void NPC::Print(std::ostream& os) const {
  std::shared_lock<std::shared_mutex> lock(mutex_);
  os << NpcStats::GetTypeName(type_) << " \"" << name_ << "\" at (" 
     << x_ << ", " << y_ << ")";
  if (!alive_) {
    os << " [DEAD]";
  }
}

void NPC::Save(std::ostream& os) const {
  std::shared_lock<std::shared_mutex> lock(mutex_);
  os << static_cast<int>(type_) << " " << name_ << " " << x_ << " " << y_;
}

std::ostream& operator<<(std::ostream& os, const NPC& npc) {
  npc.Print(os);
  return os;
}
