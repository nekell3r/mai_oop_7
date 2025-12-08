#include "observer.hpp"

#include <iostream>
#include <string_view>

#include "npc.hpp"
#include "npc_types.hpp"

namespace lab7 {

namespace {

constexpr std::string_view kMurderPrefix = "MURDER: ";

const char* NpcTypeToString(NpcType type) {
  return NpcStats::GetTypeName(type);
}

}  // namespace

std::mutex ConsoleObserver::cout_mutex_;

void ConsoleObserver::OnFight(const std::shared_ptr<NPC>& attacker,
                               const std::shared_ptr<NPC>& defender,
                               bool win) {
  if (win) {
    std::lock_guard<std::mutex> lock(cout_mutex_);
    std::cout << kMurderPrefix << NpcTypeToString(attacker->GetType())
              << " \"" << attacker->GetName() << "\" killed "
              << NpcTypeToString(defender->GetType()) 
              << " \"" << defender->GetName() << "\"" << std::endl;
  }
}

FileObserver::FileObserver(const std::string& filename) 
    : log_file_(filename, std::ios::app) {}

FileObserver::~FileObserver() {
  if (log_file_.is_open()) {
    log_file_.close();
  }
}

void FileObserver::OnFight(const std::shared_ptr<NPC>& attacker,
                            const std::shared_ptr<NPC>& defender,
                            bool win) {
  if (win && log_file_.is_open()) {
    log_file_ << kMurderPrefix << NpcTypeToString(attacker->GetType()) 
              << " \"" << attacker->GetName() << "\" killed "
              << NpcTypeToString(defender->GetType()) 
              << " \"" << defender->GetName() << "\"" << std::endl;
  }
}

}  // namespace lab7
