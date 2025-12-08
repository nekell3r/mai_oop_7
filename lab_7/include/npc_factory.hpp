#pragma once

#include <istream>
#include <memory>
#include <string>
#include <vector>

#include "npc.hpp"

namespace lab7 {

class NpcFactory {
 public:
  static std::shared_ptr<NPC> CreateNPC(NpcType type, 
                                        const std::string& name, 
                                        int x, 
                                        int y);

  static std::shared_ptr<NPC> CreateNPC(std::istream& is);

  static void SaveToFile(const std::vector<std::shared_ptr<NPC>>& npcs,
                         const std::string& filename);

  static std::vector<std::shared_ptr<NPC>> LoadFromFile(const std::string& filename);
};

}  // namespace lab7

