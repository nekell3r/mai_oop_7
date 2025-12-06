#pragma once

#include <memory>
#include <fstream>
#include <iostream>
#include <mutex>

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

