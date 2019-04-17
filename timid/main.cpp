#include <iostream>
#include <random>

#include <boost/range/adaptors.hpp>
#include <boost/range/irange.hpp>
#include <boost/range/numeric.hpp>

#include "../liars-dice/program.hpp"

class timid: public liars_dice::program {
  std::mt19937_64 _random_engine;

public:
  timid() noexcept: _random_engine(std::random_device()()) {
    ;
  }

  void check_other_programs(const std::vector<liars_dice::career>& careers) noexcept {
    ;
  }

  liars_dice::action action(const liars_dice::game& game) noexcept {
    if (std::empty(game.players()[game.previous_player_index()].actions())) {
      return liars_dice::action(liars_dice::bid(2, 1));
    }

    const auto& action_candidate = [&]() {
      const auto& faces = game.players()[game.player_index()].faces();

      const auto& secret_dice_count = (
        boost::accumulate(game.players() | boost::adaptors::transformed([](const auto& player) { return std::size(player.faces()); }), 0) -
        static_cast<int>(std::size(faces)));

      const auto& previous_bid = game.players()[game.previous_player_index()].actions().back().bid().value();

      if (std::round(secret_dice_count / 3.0f) + game.face_count(previous_bid.face()) >= previous_bid.min_count() + 1 || previous_bid.face() == 6) {
        return liars_dice::action(liars_dice::bid(previous_bid.face(), previous_bid.min_count() + 1));
      }

      return liars_dice::action(liars_dice::bid(previous_bid.face() + 1, previous_bid.min_count()));
    }();

    if (!game.is_legal_action(action_candidate)) {
      return liars_dice::action(liars_dice::challenge());
    }

    return action_candidate;
  }

  void game_end(const liars_dice::game& game) noexcept {
    ;
  }
};

int main(int argc, char** argv) {
  timid().execute();

  return 0;
}
