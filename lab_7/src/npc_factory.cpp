#include "npc_factory.hpp"

#include <fstream>
#include <stdexcept>

#include "bear.hpp"
#include "elf.hpp"
#include "robber.hpp"

std::shared_ptr<NPC> NpcFactory::CreateNPC(NpcType type,
                                            const std::string& name,
                                            int x,
                                            int y) {
  if (x < 0 || x > 100 || y < 0 || y > 100) {
    throw std::invalid_argument("Coordinates must be in range [0, 100]");
  }

  switch (type) {
    case NpcType::Bear:
      return std::make_shared<Bear>(name, x, y);
    case NpcType::Elf:
      return std::make_shared<Elf>(name, x, y);
    case NpcType::Robber:
      return std::make_shared<Robber>(name, x, y);
    default:
      throw std::invalid_argument("Unknown NPC type");
  }
}

std::shared_ptr<NPC> NpcFactory::CreateNPC(std::istream& is) {
  int type_int;
  std::string name;
  int x, y;

  if (!(is >> type_int >> name >> x >> y)) {
    return nullptr;
  }

  auto type = static_cast<NpcType>(type_int);
  return CreateNPC(type, name, x, y);
}

void NpcFactory::SaveToFile(const std::vector<std::shared_ptr<NPC>>& npcs,
                             const std::string& filename) {
  std::ofstream ofs(filename);
  if (!ofs.is_open()) {
    throw std::runtime_error("Cannot open file for writing: " + filename);
  }

  ofs << npcs.size() << std::endl;
  for (const auto& npc : npcs) {
    npc->Save(ofs);
    ofs << std::endl;
  }
}

std::vector<std::shared_ptr<NPC>> NpcFactory::LoadFromFile(
    const std::string& filename) {
  std::ifstream ifs(filename);
  if (!ifs.is_open()) {
    throw std::runtime_error("Cannot open file for reading: " + filename);
  }

  std::vector<std::shared_ptr<NPC>> result;
  size_t count;

  if (!(ifs >> count)) {
    return result;
  }

  for (size_t i = 0; i < count; ++i) {
    auto npc = CreateNPC(ifs);
    if (npc) {
      result.push_back(npc);
    }
  }

  return result;
}
