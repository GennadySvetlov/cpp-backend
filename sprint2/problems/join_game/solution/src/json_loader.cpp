#include "json_loader.h"

//#include <boost/json.hpp>

#include <iostream>
#include <string>
#include <fstream>

namespace json_loader {

model::Game LoadGame(const std::filesystem::path& json_path) {
    // Загрузить содержимое файла json_path, например, в виде строки
    // Распарсить строку как JSON, используя boost::json::parse
    // Загрузить модель игры из файла
    model::Game game;
    std::string json_str, line;
    std::ifstream in(json_path);
    if (in.is_open())
    {
        while (getline(in, line))
        {
            json_str += line;
        }
    }else {
        throw std::ifstream::failure("File open error");
    }
    in.close();

    ParseMaps(json_str, game);
    return game;
}

std::string SerializeMaps(const model::Game &game)
{
    auto maps = game.GetMaps();
    json::array maps_json;
    for(auto map : maps){
        json::object map_json;
        map_json.emplace("id" , map.GetId().operator*());
        map_json.emplace("name" , map.GetName());
        maps_json.push_back(map_json);
    }
    return json::serialize(maps_json);
}

std::string SerializeMap(const model::Game &game, const std::string &map_id)
{
    if (game.FindMap(model::Map::Id{map_id}) == nullptr){
        return "mapNotFound";
    }
    auto map = game.FindMap(model::Map::Id{map_id});
    json::object map_json;
    map_json.emplace("id", map->GetId().operator*());
    map_json.emplace("name", map->GetName());
    map_json.emplace("roads", GetRoadsJson(map->GetRoads()));
    map_json.emplace("buildings", GetBuildingsJson(map->GetBuildings()));
    map_json.emplace("offices", GetOfficesJson(map->GetOffices()));

    return json::serialize(map_json);
}

std::string SerializeError(std::string err)
{
    json::object err_json;
    err_json.emplace("code", err);
    if (err == "mapNotFound")
        err_json.emplace("message", "Map not found");
    if (err == "badRequest")
        err_json.emplace("message", "Bad request");
    return json::serialize(err_json);
}

void ParseMaps(const std::string &json_str, model::Game &game)
{
    auto value = json::parse(json_str);
    for(auto map_obj : value.as_object().at("maps").as_array()){
        auto id_str = static_cast<std::string>(map_obj.as_object().at("id").as_string());
        auto name = static_cast<std::string>(map_obj.as_object().at("name").as_string());
        model::Map::Id id{id_str};
        auto map = model::Map(id, name);

        ParseRoads(map_obj.as_object().at("roads").as_array(), map);
        ParseBuildings(map_obj.as_object().at("buildings").as_array(), map);
        ParseOffices(map_obj.as_object().at("offices").as_array(), map);

        game.AddMap(map);
    }
}

void ParsePlayer(const std::string &json_str, model::Game &game)
{
    auto value = json::parse(json_str);
    for(auto map_obj : value.as_object().at("maps").as_array()){
        auto id_str = static_cast<std::string>(map_obj.as_object().at("id").as_string());
        auto name = static_cast<std::string>(map_obj.as_object().at("name").as_string());
        model::Map::Id id{id_str};
        auto map = model::Map(id, name);

        ParseRoads(map_obj.as_object().at("roads").as_array(), map);
        ParseBuildings(map_obj.as_object().at("buildings").as_array(), map);
        ParseOffices(map_obj.as_object().at("offices").as_array(), map);

        game.AddMap(map);
    }
}

void ParseRoads(const json::array &roads, model::Map &map)
{
    for(auto road_obj : roads){
        int x0 = road_obj.as_object().at("x0").as_int64();
        int y0 = road_obj.as_object().at("y0").as_int64();
        if(road_obj.as_object().contains("x1")){
            int x1 = road_obj.as_object().at("x1").as_int64();
            map.AddRoad({model::Road::HORIZONTAL, {x0, y0}, x1});
        }else if(road_obj.as_object().contains("y1")){
            int y1 = road_obj.as_object().at("y1").as_int64();
            map.AddRoad({model::Road::VERTICAL, {x0, y0}, y1});
        }
    }
}

void ParseBuildings(const json::array &buildings, model::Map &map)
{
    for(auto building_obj : buildings){
        int x = building_obj.as_object().at("x").as_int64();
        int y = building_obj.as_object().at("y").as_int64();
        int w = building_obj.as_object().at("w").as_int64();
        int h = building_obj.as_object().at("h").as_int64();
        model::Building build({{x, y}, {w, h}});
        map.AddBuilding(build);
    }
}

void ParseOffices(const json::array &offices, model::Map &map)
{
    for(auto office_obj : offices){
        auto id_office_str = static_cast<std::string>(office_obj.as_object().at("id").as_string());
        int x = office_obj.as_object().at("x").as_int64();
        int y = office_obj.as_object().at("y").as_int64();
        int offsetX = office_obj.as_object().at("offsetX").as_int64();
        int offsetY = office_obj.as_object().at("offsetY").as_int64();
        model::Office::Id id_office{id_office_str};
        map.AddOffice({id_office, {x, y}, {offsetX, offsetY}});
    }
}

json::array GetRoadsJson(const model::Map::Roads &roads)
{
    json::array roads_json;
    for(auto road : roads){
        json::object road_json;
        auto start = road.GetStart();
        auto end = road.GetEnd();
        road_json.emplace("x0", start.x);
        road_json.emplace("y0", start.y);
        if(road.IsHorizontal()){
            road_json.emplace("x1", end.x);
        }else{
            road_json.emplace("y1", end.y);
        }
        roads_json.push_back(road_json);
    }
    return roads_json;
}

json::array GetBuildingsJson(const model::Map::Buildings &buildings)
{
    json::array buildings_json;
    for(auto building : buildings){
        json::object building_json;
        auto position = building.GetBounds().position;
        auto size = building.GetBounds().size;
        building_json.emplace("x", position.x);
        building_json.emplace("y", position.y);
        building_json.emplace("w", size.width);
        building_json.emplace("h", size.height);
        buildings_json.push_back(building_json);
    }
    return buildings_json;
}

json::array GetOfficesJson(const model::Map::Offices &offices)
{
    json::array offices_json;
    for(auto office : offices){
        json::object office_json;
        auto position = office.GetPosition();
        auto offset = office.GetOffset();
        office_json.emplace("id", office.GetId().operator*());
        office_json.emplace("x", position.x);
        office_json.emplace("y", position.y);
        office_json.emplace("offsetX", offset.dx);
        office_json.emplace("offsetY", offset.dy);
        offices_json.push_back(office_json);
    }
    return offices_json;
}

json::value BuildLog(std::string ts, json::value data, std::string message)
{
    json::object log;
    log.emplace("timestamp", ts);
    log.emplace("data", data);
    log.emplace("message", message);
    return log;
}

}  // namespace json_loader
