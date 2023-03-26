#pragma once

#include <filesystem>

#include <boost/json.hpp>

#include "model.h"

namespace json_loader {

namespace json = boost::json;
using namespace std::literals;

model::Game LoadGame(const std::filesystem::path& json_path);

void ParseMaps(const std::string& json_str, model::Game& game);

void ParseRoads(const json::array& roads, model::Map& map);

void ParseBuildings(const json::array& buildings, model::Map& map);

void ParseOffices(const json::array& offices, model::Map& map);

void ParsePlayer(const std::string& json_str, model::Game& game);

std::string SerializeMaps(const model::Game& game);

std::string SerializeMap(const model::Game& game, const std::string& map);

json::array GetRoadsJson(const model::Map::Roads& roads);

json::array GetBuildingsJson(const model::Map::Buildings& buildings);

json::array GetOfficesJson(const model::Map::Offices& offices);

std::string SerializeError(std::string err);

json::value BuildLog(std::string ts, json::value data, std::string message);

}  // namespace json_loader
