#pragma once

#include "npc.hpp"

namespace lab7::NpcStats {

constexpr int GetMoveDistance(NpcType type) {
  switch (type) {
    case NpcType::Bear:
      return 5;
    case NpcType::Elf:
      return 10;
    case NpcType::Robber:
      return 10;
    case NpcType::Unknown:
      break;
  }
  return 0;
}

constexpr int GetKillDistance(NpcType type) {
  switch (type) {
    case NpcType::Bear:
      return 10;
    case NpcType::Elf:
      return 50;
    case NpcType::Robber:
      return 10;
    case NpcType::Unknown:
      break;
  }
  return 0;
}

constexpr const char* GetTypeName(NpcType type) {
  switch (type) {
    case NpcType::Bear:
      return "Bear";
    case NpcType::Elf:
      return "Elf";
    case NpcType::Robber:
      return "Robber";
    case NpcType::Unknown:
      break;
  }
  return "Unknown";
}

}  // namespace lab7::NpcStats
