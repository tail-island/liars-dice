#pragma once

#include <functional>
#include <string>
#include <vector>

#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "game.hpp"

namespace liars_dice {
  struct career_record final {
    std::string id;
    liars_dice::game game;
  };

  struct career final {
    std::string id;
    std::vector<career_record> career_records;
  };

  // object -> json

  inline auto write_bid(const bid& bid, rapidjson::Writer<rapidjson::StringBuffer>& writer) noexcept {
    writer.StartObject();
    writer.Key("face"); writer.Int(bid.face());
    writer.Key("min_count"); writer.Int(bid.min_count());
    writer.EndObject();
  }

  inline auto write_challenge(const challenge& challenge, rapidjson::Writer<rapidjson::StringBuffer>& writer) noexcept {
    writer.StartObject();
    writer.EndObject();
  }

  inline auto write_action(const action& action, rapidjson::Writer<rapidjson::StringBuffer>& writer) noexcept {
    writer.StartObject();
    if (action.bid()) {
      writer.Key("bid");
      write_bid(action.bid().value(), writer);
    }
    if (action.challenge()) {
      writer.Key("challenge");
      write_challenge(action.challenge().value(), writer);
    }
    writer.EndObject();
  }

  inline auto write_player(const player& player, rapidjson::Writer<rapidjson::StringBuffer>& writer) noexcept {
    writer.StartObject();
    writer.Key("id");
    writer.String(player.id().c_str());
    writer.Key("faces");
    writer.StartArray();
    for (const auto& face: player.faces()) {
      writer.Int(face);
    }
    writer.EndArray();
    writer.Key("actions");
    writer.StartArray();
    for (const auto& action: player.actions()) {
      write_action(action, writer);
    }
    writer.EndArray();
    writer.EndObject();
  }

  inline auto write_game(const game& game, rapidjson::Writer<rapidjson::StringBuffer>& writer) noexcept {
    writer.StartObject();
    writer.Key("players");
    writer.StartArray();
    for (const auto& player: game.players()) {
      write_player(player, writer);
    }
    writer.EndArray();
    writer.Key("player_index");
    writer.Int(game.player_index());
    writer.EndObject();
  }

  inline auto write_career_record(const career_record& career_record, rapidjson::Writer<rapidjson::StringBuffer>& writer) noexcept {
    writer.StartObject();
    writer.Key("id");
    writer.String(career_record.id.c_str());
    writer.Key("game");
    write_game(career_record.game, writer);
    writer.EndObject();
  }

  inline auto write_career(const career& career, rapidjson::Writer<rapidjson::StringBuffer>& writer) noexcept {
    writer.StartObject();
    writer.Key("id");
    writer.String(career.id.c_str());
    writer.Key("career_records");
    writer.StartArray();
    for (const auto& career_record: career.career_records) {
      write_career_record(career_record, writer);
    }
    writer.EndArray();
    writer.EndObject();
  }

  inline auto write_careers(const std::vector<career>& careers, rapidjson::Writer<rapidjson::StringBuffer>& writer) noexcept {
    writer.StartArray();
    for (const auto& career: careers) {
      write_career(career, writer);
    }
    writer.EndArray();
  }

  template<class T>
  inline auto write_json(const T& t, const std::function<void(const T&, rapidjson::Writer<rapidjson::StringBuffer>&)>& write_t) noexcept {
    auto string_buffer = rapidjson::StringBuffer();
    auto writer = rapidjson::Writer<rapidjson::StringBuffer>(string_buffer);

    write_t(t, writer);

    return std::string(string_buffer.GetString());
  }

  // json -> object

  inline auto read_bid(const rapidjson::Value& value) noexcept {
    const auto& face = value["face"].GetInt();
    const auto& min_count = value["min_count"].GetInt();

    return bid(face, min_count);
  }

  inline auto read_challenge(const rapidjson::Value& value) noexcept {
    return challenge();
  }

  inline auto read_action(const rapidjson::Value& value) {
    if (value.HasMember("bid") && !value["bid"].IsNull()) {
      const auto& bid = read_bid(value["bid"]);

      return action(bid);
    }

    if (value.HasMember("challenge") && !value["challenge"].IsNull()) {
      const auto& challenge = read_challenge(value["challenge"]);

      return action(challenge);
    }

    throw std::exception();
  }

  inline auto read_player(const rapidjson::Value& value) noexcept {
    const auto& id = value["id"].GetString();
    const auto& faces = [&]() {
      auto result = std::vector<int>();

      for (auto it = value["faces"].Begin(); it != value["faces"].End(); ++it) {
        result.emplace_back(it->GetInt());
      }

      return result;
    }();
    const auto& actions = [&]() {
      auto result = std::vector<action>();

      for (auto it = value["actions"].Begin(); it != value["actions"].End(); ++it) {
        result.emplace_back(read_action(*it));
      }

      return result;
    }();

    return player(id, faces, actions);
  }

  inline auto read_game(const rapidjson::Value& value) noexcept {
    const auto& players = [&]() {
      auto result = std::vector<player>();

      for (auto it = value["players"].Begin(); it != value["players"].End(); ++it) {
        result.emplace_back(read_player(*it));
      }

      return result;
    }();
    const auto& player_index = value["player_index"].GetInt();

    return game(players, player_index);
  }

  inline auto read_career_record(const rapidjson::Value& value) noexcept {
    const auto& id = value["id"].GetString();
    const auto& game = read_game(value["game"]);

    return career_record{id, game};
  }

  inline auto read_career(const rapidjson::Value& value) noexcept {
    const auto& id = value["id"].GetString();
    const auto& career_records = [&]() {
      auto result = std::vector<career_record>();

      for (auto it = value["career_records"].Begin(); it != value["career_records"].End(); ++it) {
        result.emplace_back(read_career_record(*it));
      }

      return result;
    }();

    return career{id, career_records};
  }

  inline auto read_careers(const rapidjson::Value& value) noexcept {
    auto result = std::vector<career>();

    for (auto it = value.Begin(); it != value.End(); ++it) {
      result.emplace_back(read_career(*it));
    }

    return result;
  }

  template<class T>
  inline auto read_json(const std::string& json, const std::function<T(const rapidjson::Value& value)>& read_t) noexcept {
    auto document = rapidjson::Document();

    document.Parse(json.c_str());

    return read_t(document);
  }
}
