#include "game.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <random>
#include <thread>

#include "fight_visitor.hpp"
#include "npc_factory.hpp"
#include "observer.hpp"

namespace lab7 {
namespace {

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> coord_dist(0, 100);
std::uniform_int_distribution<> type_dist(1, 3);

}  // namespace

Game::Game() : running_(false) {}

Game::~Game() {
  Stop();
}

void Game::Initialize(int npc_count) {
  std::unique_lock<std::shared_mutex> lock(npcs_mutex_);
  
  npcs_.clear();
  npcs_.reserve(npc_count);
  
  auto console_obs = std::make_shared<ConsoleObserver>();
  auto file_obs = std::make_shared<FileObserver>("log.txt");
  
  for (int i = 0; i < npc_count; ++i) {
    auto type = static_cast<NpcType>(type_dist(gen));
    std::string name = "NPC" + std::to_string(i);
    int x = coord_dist(gen);
    int y = coord_dist(gen);
    
    auto npc = NpcFactory::CreateNPC(type, name, x, y);
    npc->Subscribe(console_obs);
    npc->Subscribe(file_obs);
    npcs_.push_back(npc);
  }
}

void Game::MovementThread() {
  std::random_device rd_local;
  std::mt19937 gen_local(rd_local());
  std::uniform_int_distribution<> move_dist(-1, 1);
  
  while (running_) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::shared_lock<std::shared_mutex> read_lock(npcs_mutex_);
    auto npcs_copy = npcs_;
    read_lock.unlock();
    
    for (auto& npc : npcs_copy) {
      if (!npc->IsAlive()) continue;
      
      int move_dist = npc->GetMoveDistance();
      int current_x = npc->GetX();
      int current_y = npc->GetY();
      
      std::uniform_real_distribution<double> angle_dist(0.0, 2.0 * 3.14159265359);
      double angle = angle_dist(gen_local);
      
      int new_x = current_x + static_cast<int>(move_dist * std::cos(angle));
      int new_y = current_y + static_cast<int>(move_dist * std::sin(angle));
      
      npc->Move(new_x, new_y);
      
      if (!npc->IsAlive()) continue;
      
      int kill_dist = npc->GetKillDistance();
      
      for (auto& other : npcs_copy) {
        if (npc == other) continue;
        if (!other->IsAlive()) continue;
        if (!npc->IsAlive()) break;
        
        if (npc->IsClose(other, kill_dist)) {
          CombatTask task;
          task.attacker = npc;
          task.defender = other;
          
          {
            std::lock_guard<std::mutex> queue_lock(combat_queue_mutex_);
            if (combat_queue_.size() < 500) {
              combat_queue_.push(task);
              combat_queue_cv_.notify_one();
            }
          }
        }
      }
    }
  }
}

void Game::CombatThread() {
  while (running_) {
    std::unique_lock<std::mutex> queue_lock(combat_queue_mutex_);
    combat_queue_cv_.wait(queue_lock, [this] { 
      return !combat_queue_.empty() || !running_; 
    });
    
    if (!running_ && combat_queue_.empty()) break;
    
    if (combat_queue_.empty()) continue;
    
    CombatTask task = combat_queue_.front();
    combat_queue_.pop();
    queue_lock.unlock();
    
    bool attacker_alive = task.attacker->IsAlive();
    bool defender_alive = task.defender->IsAlive();
    
    if (!attacker_alive || !defender_alive) {
      continue;
    }
    
    auto attacker_visitor = std::dynamic_pointer_cast<FightVisitor>(task.attacker);
    bool defender_killed = false;
    if (attacker_visitor && task.defender->Accept(attacker_visitor)) {
      task.defender->Kill();
      defender_killed = true;
      task.attacker->FightNotify(task.attacker, task.defender, true);
      
      {
        std::lock_guard<std::mutex> cout_lock(cout_mutex_);
        std::cout << "COMBAT: " << task.attacker->GetName() 
                  << " killed " << task.defender->GetName() << std::endl;
      }
    }
    
    if (task.attacker->IsAlive() && !defender_killed) {
      auto defender_visitor = std::dynamic_pointer_cast<FightVisitor>(task.defender);
      if (defender_visitor && task.defender->IsAlive() && task.attacker->Accept(defender_visitor)) {
        task.attacker->Kill();
        task.defender->FightNotify(task.defender, task.attacker, true);
        
        {
          std::lock_guard<std::mutex> cout_lock(cout_mutex_);
          std::cout << "COMBAT: " << task.defender->GetName() 
                    << " killed " << task.attacker->GetName() << std::endl;
        }
      }
    }
  }
}

void Game::MainThread() {
  auto start_time = std::chrono::steady_clock::now();
  
  while (running_) {
    auto current_time = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
        current_time - start_time).count();
    
    if (elapsed >= GAME_DURATION_SECONDS) {
      running_ = false;
      break;
    }
    
    PrintMap();
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  
  {
    std::lock_guard<std::mutex> cout_lock(cout_mutex_);
    std::cout << "\n=== Game Over ===" << std::endl;
    auto survivors = GetAliveNPCs();
    std::cout << "Survivors: " << survivors.size() << std::endl;
    for (const auto& npc : survivors) {
      std::cout << "  " << *npc << std::endl;
    }
  }
}

void Game::Run() {
  running_ = true;
  
  std::thread movement_thread(&Game::MovementThread, this);
  std::thread combat_thread(&Game::CombatThread, this);
  std::thread main_thread(&Game::MainThread, this);
  
  main_thread.join();
  running_ = false;
  combat_queue_cv_.notify_all();
  
  movement_thread.join();
  combat_thread.join();
}

void Game::Stop() {
  running_ = false;
  combat_queue_cv_.notify_all();
}

std::vector<std::shared_ptr<NPC>> Game::GetAliveNPCs() const {
  std::shared_lock<std::shared_mutex> lock(npcs_mutex_);
  std::vector<std::shared_ptr<NPC>> alive;
  
  for (const auto& npc : npcs_) {
    if (npc->IsAlive()) {
      alive.push_back(npc);
    }
  }
  
  return alive;
}

void Game::PrintMap() const {
  std::lock_guard<std::mutex> cout_lock(cout_mutex_);
  
  std::cout << "\n=== Map ===" << std::endl;
  
  std::shared_lock<std::shared_mutex> lock(npcs_mutex_);
  for (const auto& npc : npcs_) {
    if (npc->IsAlive()) {
      std::cout << *npc << std::endl;
    }
  }
}

}  // namespace lab7

