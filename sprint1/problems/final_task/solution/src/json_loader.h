#pragma once

#include <filesystem>

#include "model.h"

namespace json_loader {

model::Game LoadGame(const std::filesystem::path& json_path);

std::string SerializeMaps(const model::Game& game); //SerializeMap

std::string SerializeMap(const model::Game& game, const std::string& map);

std::string SerializeError(std::string err);



}  // namespace json_loader
