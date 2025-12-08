#pragma once

#include <fstream>
#include <memory>
#include <mutex>
#include <string>

namespace lab7 {

class NPC;

class IFightObserver {
 public:
  virtual ~IFightObserver() = default;
  virtual void OnFight(const std::shared_ptr<NPC>& attacker,
                       const std::shared_ptr<NPC>& defender,
                       bool win) = 0;
};

class ConsoleObserver : public IFightObserver {
 private:
  static std::mutex cout_mutex_;

 public:
  void OnFight(const std::shared_ptr<NPC>& attacker,
               const std::shared_ptr<NPC>& defender,
               bool win) override;
};

class FileObserver : public IFightObserver {
 private:
  std::ofstream log_file_;

 public:
  explicit FileObserver(const std::string& filename);
  ~FileObserver();

  void OnFight(const std::shared_ptr<NPC>& attacker,
               const std::shared_ptr<NPC>& defender,
               bool win) override;
};

}  // namespace lab7

