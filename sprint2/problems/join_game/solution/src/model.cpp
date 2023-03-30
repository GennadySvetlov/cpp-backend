#include "model.h"

#include <stdexcept>

namespace model {
using namespace std::literals;

std::uint64_t Dog::global_dog_id = 0;
std::uint64_t Player::global_player_id = 0;
const int maxDogsCount = 5;

void Map::AddOffice(Office office) {
    if (warehouse_id_to_index_.contains(office.GetId())) {
        throw std::invalid_argument("Duplicate warehouse");
    }

    const size_t index = offices_.size();
    Office& o = offices_.emplace_back(std::move(office));
    try {
        warehouse_id_to_index_.emplace(o.GetId(), index);
    } catch (...) {
        // Удаляем офис из вектора, если не удалось вставить в unordered_map
        offices_.pop_back();
        throw;
    }
}

void Game::AddMap(Map map) {
    const size_t index = maps_.size();
    if (auto [it, inserted] = map_id_to_index_.emplace(map.GetId(), index); !inserted) {
        throw std::invalid_argument("Map with id "s + *map.GetId() + " already exists"s);
    } else {
        try {
            maps_.emplace_back(std::move(map));
        } catch (...) {
            map_id_to_index_.erase(it);
            throw;
        }
    }
}

void Game::AddPlayer(std::string playerName, std::string mapId)
{

}

const Map &Game::CreateSession(const Map::Id &id)
{
    auto map = GetRefMap(Map::Id{id});
    gamesessions_.emplace_back(GameSession{map});
    return gamesessions_[gamesessions_.size() - 1].GetMap();
}

bool GameSession::IsFull()
{
    return dogs_.size() == maxDogsCount;
}

void GameSession::AddDog(Dog dog)
{
    if (dog_id_to_index_.contains(dog.GetId())) {
        throw std::invalid_argument("Duplicate dog");
    }

    const size_t index = dogs_.size();
    Dog& o = dogs_.emplace_back(std::move(dog));
    try {
        dog_id_to_index_.emplace(o.GetId(), index);
    } catch (...) {
        // Удаляем собаку из вектора, если не удалось вставить в unordered_map
        dogs_.pop_back();
        throw;
    }
}

}  // namespace model
