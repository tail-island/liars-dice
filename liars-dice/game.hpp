#pragma once

#include <optional>
#include <unordered_map>
#include <vector>

#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/irange.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "util.hpp"

namespace liars_dice {
  class bid final {
    int _face;
    int _min_count;

  public:
    bid(int face, int min_count) noexcept: _face(face), _min_count(min_count) {
      ;
    }

    const auto& face() const noexcept {
      return _face;
    }

    const auto& min_count() const noexcept {
      return _min_count;
    }
  };

  class challenge final {
    ;
  };

  class action final {
    std::optional<liars_dice::bid> _bid;
    std::optional<liars_dice::challenge> _challenge;

  public:
    action(const bid& bid) noexcept: _bid(bid), _challenge(std::nullopt) {
      ;
    }

    action(const challenge& challenge) noexcept: _bid(std::nullopt), _challenge(challenge) {
      ;
    }

    const auto& bid() const noexcept {
      return _bid;
    }

    const auto& challenge() const noexcept {
      return _challenge;
    }
  };

  class player final {
    std::string _id;
    std::vector<int> _faces;
    std::vector<action> _actions;

  public:
    player(const std::string& id, const std::vector<int>& faces, const std::vector<action>& actions) noexcept: _id(id), _faces(faces), _actions(actions) {
      ;
    }

    player(const std::string& id, const std::vector<int>& faces) noexcept: player(id, faces, {}) {
      ;
    }

    const auto& id() const noexcept {
      return _id;
    }

    const auto& faces() const noexcept {
      return _faces;
    }

    auto& faces() noexcept {
      return _faces;
    }

    const auto& actions() const noexcept {
      return _actions;
    }

    auto& actions() noexcept {
      return _actions;
    }
  };

  class game final {
    std::vector<player> _players;
    int _player_index;

  public:
    game(const std::vector<player>& players, int player_index) noexcept: _players(players), _player_index(player_index) {
      ;
    }

    game(const std::vector<player>& players) noexcept: game(players, 0) {
      ;
    }

    const auto& players() const noexcept {
      return _players;
    }

    auto& players() noexcept {
      return _players;
    }

    const auto& player_index() const noexcept {
      return _player_index;
    }

    auto previous_player_index() const noexcept {
      return (player_index() + static_cast<int>(std::size(players())) - 1) % static_cast<int>(std::size(players()));
    }

    auto masked_game() const noexcept {
      auto result = *this;

      for (const auto& i: boost::irange(0, static_cast<int>(std::size(result.players())))) {
        if (i != result.player_index()) {
          result.players()[i].faces() = std::vector<int>(std::size(result.players()[i].faces()), 0);
        }
      }

      return result;
    }

    auto face_count(int target_face) const noexcept {
      auto result = 0;

      for (const auto& player: players()) {
        for (const auto& face: player.faces()) {
          if (face == target_face || face == 1) {
            result++;
          }
        }
      }

      return result;
    }

    auto is_legal_action(const action& action) const noexcept {
      if (action.bid()) {
        const auto& bid = action.bid().value();

        if (bid.face() < 2 || bid.face() > 6) {
          return false;
        }

        if (bid.min_count() < 1) {
          return false;
        }

        if (bid.min_count() > 20) {
          return false;
        }

        if (!std::empty(players()[previous_player_index()].actions())) {
          const auto& previous_bid = players()[previous_player_index()].actions().back().bid().value();

          if (bid.face() <= previous_bid.face() && bid.min_count() <= previous_bid.min_count()) {
            return false;
          }

          if (bid.face() >  previous_bid.face() && bid.min_count() <  previous_bid.min_count()) {
            return false;
          }
        }

        return true;
      }

      if (action.challenge()) {
        if (std::empty(players()[previous_player_index()].actions())) {
          return false;
        }

        return true;
      }

      return false;
    }

    auto do_action(const action& action) noexcept {
      _players[player_index()].actions().emplace_back(action);

      if (action.challenge()) {
        return;
      }

      _player_index = (player_index() + 1) % std::size(players());
    }

    auto is_end() const noexcept {
      return !std::empty(players()[player_index()].actions()) && players()[player_index()].actions().back().challenge();
    }

    auto dice_count_deltas() const noexcept {
      auto result = std::vector<int>(std::size(players()), 0);

      const auto& bid = players()[previous_player_index()].actions().back().bid().value();
      const auto& face_count = game::face_count(bid.face());

      if (face_count < bid.min_count()) {
        result[previous_player_index()] = face_count - bid.min_count();

        return result;
      }

      if (face_count > bid.min_count()) {
        result[player_index()         ] = bid.min_count() - face_count;

        return result;
      }

      for (const auto& i: boost::irange(0, static_cast<int>(std::size(players())))) {
        if (i != previous_player_index()) {
          result[i] = -1;
        }
      }

      return result;
    }
  };

  inline auto play_game(const std::vector<std::string>& ids, const std::vector<int>& dice_counts, const std::vector<std::function<action(const game&)>>& action_functions) noexcept {
    auto random_engine = std::mt19937_64(std::random_device()());

    const auto& id_and_action_functions = boost::copy_range<std::unordered_map<std::string, std::function<action(const liars_dice::game&)>>>(
      util::combine(ids, action_functions) |
      boost::adaptors::transformed(
        [&](const auto& id_and_action_function) {
          const auto& [id, action_function] = id_and_action_function;

          return std::make_pair(id, action_function);
        }));

    auto game = [&]() {
      const auto& players = boost::copy_range<std::vector<player>>(
        util::combine(ids, dice_counts) |
        boost::adaptors::transformed(
          [&](const auto& id_and_dice_count) {
            const auto&[id, dice_count] = id_and_dice_count;
            const auto& faces = [&, dice_count = dice_count]() {  // P0588R1...
              auto result = boost::copy_range<std::vector<int>>(
                boost::irange(0, dice_count) |
                boost::adaptors::transformed([&](const auto& _) { return std::uniform_int_distribution(1, 6)(random_engine); }));

              boost::sort(result);

              return result;
            }();

            return player(id, faces);
          }));

      return liars_dice::game(players);
    }();

    const auto& dice_count_deltas = [&]() {
      while (!game.is_end()) {
        try {
          const auto& action = id_and_action_functions.at(game.players()[game.player_index()].id())(game.masked_game());

          if (!game.is_legal_action(action)) {
            auto result = std::vector<int>(std::size(game.players()), 0);

            result[game.player_index()] = -91;

            return result;
          }

          game.do_action(action);

        } catch (...) {
          auto result = std::vector<int>(std::size(game.players()), 0);

          result[game.player_index()] = -92;

          return result;
        }
      }

      return game.dice_count_deltas();
    }();

    return std::make_tuple(game, dice_count_deltas);
  }
}
