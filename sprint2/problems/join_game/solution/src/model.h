#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <random>
#include <iomanip>
#include <map>

#include "tagged.h"

namespace model {

using Dimension = int;
using Coord = Dimension;

struct Point {
    Coord x, y;
};

struct Size {
    Dimension width, height;
};

struct Rectangle {
    Point position;
    Size size;
};

struct Offset {
    Dimension dx, dy;
};

class Road {
    struct HorizontalTag {
        explicit HorizontalTag() = default;
    };

    struct VerticalTag {
        explicit VerticalTag() = default;
    };

public:
    constexpr static HorizontalTag HORIZONTAL{};
    constexpr static VerticalTag VERTICAL{};

    Road(HorizontalTag, Point start, Coord end_x) noexcept
        : start_{start}
        , end_{end_x, start.y} {
    }

    Road(VerticalTag, Point start, Coord end_y) noexcept
        : start_{start}
        , end_{start.x, end_y} {
    }

    bool IsHorizontal() const noexcept {
        return start_.y == end_.y;
    }

    bool IsVertical() const noexcept {
        return start_.x == end_.x;
    }

    Point GetStart() const noexcept {
        return start_;
    }

    Point GetEnd() const noexcept {
        return end_;
    }

private:
    Point start_;
    Point end_;
};

class Building {
public:
    explicit Building(Rectangle bounds) noexcept
        : bounds_{bounds} {
    }

    const Rectangle& GetBounds() const noexcept {
        return bounds_;
    }

private:
    Rectangle bounds_;
};

class Office {
public:
    using Id = util::Tagged<std::string, Office>;

    Office(Id id, Point position, Offset offset) noexcept
        : id_{std::move(id)}
        , position_{position}
        , offset_{offset} {
    }

    const Id& GetId() const noexcept {
        return id_;
    }

    Point GetPosition() const noexcept {
        return position_;
    }

    Offset GetOffset() const noexcept {
        return offset_;
    }

private:
    Id id_;
    Point position_;
    Offset offset_;
};

class Map {
public:
    using Id = util::Tagged<std::string, Map>;
    using Roads = std::vector<Road>;
    using Buildings = std::vector<Building>;
    using Offices = std::vector<Office>;

    Map(Id id, std::string name) noexcept
        : id_(std::move(id))
        , name_(std::move(name)) {
    }

    const Id& GetId() const noexcept {
        return id_;
    }

    const std::string& GetName() const noexcept {
        return name_;
    }

    const Buildings& GetBuildings() const noexcept {
        return buildings_;
    }

    const Roads& GetRoads() const noexcept {
        return roads_;
    }

    const Offices& GetOffices() const noexcept {
        return offices_;
    }

    void AddRoad(const Road& road) {
        roads_.emplace_back(road);
    }

    void AddBuilding(const Building& building) {
        buildings_.emplace_back(building);
    }

    void AddOffice(Office office);

private:
    using OfficeIdToIndex = std::unordered_map<Office::Id, size_t, util::TaggedHasher<Office::Id>>;

    Id id_;
    std::string name_;
    Roads roads_;
    Buildings buildings_;

    OfficeIdToIndex warehouse_id_to_index_;
    Offices offices_;
};

class Dog
{
public:
    using Id = util::Tagged<std::uint64_t, Dog>;

    static std::uint64_t global_dog_id;
    Dog(std::string name) : name_(name), id_(global_dog_id++) {}

    const Id& GetId() const noexcept {
        return id_;
    }

    const std::string& GetName() const noexcept {
        return name_;
    }

private:
    Id id_;
    std::string name_;

};

class GameSession
{
public:
    using Dogs = std::vector<Dog>;


    GameSession(Map& map): map_(map) {}

    bool IsFull();

    const Map::Id& GetMapId() const noexcept {
        return map_.GetId();
    }

    const Map& GetMap() const noexcept {
        return map_;
    }

    void AddDog(Dog dog);
private:
    using DogIdToIndex = std::unordered_map<Dog::Id, size_t, util::TaggedHasher<Dog::Id>>;
    Map& map_;
    DogIdToIndex dog_id_to_index_;
    Dogs dogs_;

};

struct TokenTag {};

using Token = util::Tagged<std::string, TokenTag>;

class Player
{
public:
    using Id = util::Tagged<std::uint64_t, Player>;
    static std::uint64_t global_player_id;
    Player(GameSession gameSession, Token token, std::string name) :
        gameSession_(gameSession),
//        dog_(dog),
        token_(token),
        id_(global_player_id++),
        name_(name)
    {}
private:
    Token token_;
    GameSession gameSession_;
    //Dog dog_;
    Id id_;
    std::string name_;
};

class PlayerToken
{
public:
    using Players = std::vector<Player>;
    PlayerToken() {}

    Token AppPlayer(Player player);

    static Token GenerateToken(const PlayerToken& obj = PlayerToken{}) {
        std::stringstream stream;
        stream << std::hex << obj.generator1_ << obj.generator2_;
        std::string result( stream.str() );
        return Token{result};
    };

private:
    std::random_device random_device_;
    std::mt19937_64 generator1_{[this] {
            std::uniform_int_distribution<std::mt19937_64::result_type> dist;
            return dist(random_device_);
                                }()};
    std::mt19937_64 generator2_{[this] {
            std::uniform_int_distribution<std::mt19937_64::result_type> dist;
            return dist(random_device_);
                                }()};
    // Чтобы сгенерировать токен, получите из generator1_ и generator2_
    // два 64-разрядных числа и, переведя их в hex-строки, склейте в одну.
    // Вы можете поэкспериментировать с алгоритмом генерирования токенов,
    // чтобы сделать их подбор ещё более затруднительным
};

class Game {
public:
    using Maps = std::vector<Map>;
    using GameSessions = std::vector<GameSession>;

    void AddMap(Map map);

    void AddPlayer(std::string playerName, std::string mapId);

    const Map& CreateSession(const Map::Id& id);

    const Maps& GetMaps() const noexcept {
        return maps_;
    }

    const Map* FindMap(const Map::Id& id) const noexcept {
        if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
            return &maps_.at(it->second);
        }
        return nullptr;
    }

    const Map& GetRefMap(const Map::Id& id) const noexcept{
        if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
            return maps_.at(it->second);
        }
    }

    const Map& GetFreeMap(const Map::Id& id) {
        for (auto gamesession : gamesessions_) {
            if (gamesession.GetMapId() == id && !gamesession.IsFull())
                return gamesession.GetMap();
        }
        return CreateSession(id);
    }


private:
    using MapIdHasher = util::TaggedHasher<Map::Id>;
    using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;
    using Players = std::map<Token, Player>;

    Players players_;
    Maps maps_;
    GameSessions gamesessions_;
    MapIdToIndex map_id_to_index_;
};

}  // namespace model
