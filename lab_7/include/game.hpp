#pragma once

#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <shared_mutex>

#include "npc.hpp"

struct CombatTask {
  std::shared_ptr<NPC> attacker;
  std::shared_ptr<NPC> defender;
};

class Game {
 private:
  std::vector<std::shared_ptr<NPC>> npcs_;
  mutable std::shared_mutex npcs_mutex_;
  
  std::queue<CombatTask> combat_queue_;
  std::mutex combat_queue_mutex_;
  std::condition_variable combat_queue_cv_;
  
  std::atomic<bool> running_;
  mutable std::mutex cout_mutex_;
  
  static constexpr int MAP_SIZE = 100;
  static constexpr int GAME_DURATION_SECONDS = 30;
  
  void MovementThread();
  void CombatThread();
  void MainThread();
  
 public:
  Game();
  ~Game();
  
  void Initialize(int npc_count = 50);
  void Run();
  void Stop();
  
  std::vector<std::shared_ptr<NPC>> GetAliveNPCs() const;
  void PrintMap() const;
};

