#include <iostream>
#include <random>

#include "../liars-dice/program.hpp"

class fool: public liars_dice::program {
  std::mt19937_64 _random_engine;

public:
  fool() noexcept: _random_engine(std::random_device()()) {
    ;
  }

  void check_other_programs(const std::vector<liars_dice::career>& careers) noexcept {
    ;
  }

  liars_dice::action action(const liars_dice::game& game) noexcept {
    if (std::empty(game.players()[game.previous_player_index()].actions())) {
      return liars_dice::action(liars_dice::bid(std::uniform_int_distribution(2, 6)(_random_engine), std::uniform_int_distribution(8, 10)(_random_engine)));
    }

    if (std::uniform_real_distribution(0.0f, 1.0f)(_random_engine) < 0.2) {
      return liars_dice::action(liars_dice::challenge());
    }

    const auto& previous_bid = game.players()[game.previous_player_index()].actions().back().bid().value();
    const auto& face = std::uniform_int_distribution(2, 6)(_random_engine);

    const auto& action = liars_dice::action(liars_dice::bid(face, previous_bid.min_count() + (face > previous_bid.face() ? 0 : 1)));

    if (!game.is_legal_action(action)) {
      return liars_dice::action(liars_dice::challenge());
    }

    return action;
  }

  void game_end(const liars_dice::game& game) noexcept {
    ;
  }
};

int main(int argc, char** argv) {
  fool().execute();

  return 0;
}
