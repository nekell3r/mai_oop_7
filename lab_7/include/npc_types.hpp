#pragma once

#include "npc.hpp"

// Table of killability and movement distances
namespace NpcStats {
  constexpr int GetMoveDistance(NpcType type) {
    switch (type) {
      case NpcType::Bear: return 5;
      case NpcType::Elf: return 10;
      case NpcType::Robber: return 10;
      default: return 0;
    }
  }

  constexpr int GetKillDistance(NpcType type) {
    switch (type) {
      case NpcType::Bear: return 10;
      case NpcType::Elf: return 50;
      case NpcType::Robber: return 10;
      default: return 0;
    }
  }

  constexpr const char* GetTypeName(NpcType type) {
    switch (type) {
      case NpcType::Bear: return "Bear";
      case NpcType::Elf: return "Elf";
      case NpcType::Robber: return "Robber";
      default: return "Unknown";
    }
  }
}
