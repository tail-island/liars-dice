#include <iostream>
#include <random>

#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include <boost/range/adaptors.hpp>
#include <boost/range/irange.hpp>
#include <boost/range/numeric.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "../liars-dice/program.hpp"

class optimist: public liars_dice::program {
  std::mt19937_64 _random_engine;

public:
  optimist() noexcept: _random_engine(std::random_device()()) {
    ;
  }

  void check_other_programs(const std::vector<liars_dice::career>& careers) noexcept {
    ;
  }

  liars_dice::action action(const liars_dice::game& game) noexcept {
    const auto& faces = game.players()[game.player_index()].faces();

    const auto& secret_dice_count = (
      boost::accumulate(game.players() | boost::adaptors::transformed([](const auto& player) { return std::size(player.faces()); }), 0) -
      static_cast<int>(std::size(faces)));

    const auto& estimated_face_counts = boost::copy_range<std::vector<int>>(
      boost::irange(2, 7) |
      boost::adaptors::transformed([&](const auto& face) { return std::round(secret_dice_count / 3.0f) + game.face_count(face) + 1; }));  // この+1が楽天的。

    const auto& action_candidates = boost::copy_range<std::vector<liars_dice::action>>(
      boost::irange(2, 7) |
      boost::adaptors::transformed([&](const auto& face) { return liars_dice::action(liars_dice::bid(face, estimated_face_counts[face - 2])); }) |
      boost::adaptors::filtered([&](const auto& action) { return game.is_legal_action(action); }));

    if (std::empty(action_candidates)) {
      return liars_dice::action(liars_dice::challenge());
    }

    return action_candidates[std::uniform_int_distribution(0, static_cast<int>(std::size(action_candidates)) - 1)(_random_engine)];
  }

  void game_end(const liars_dice::game& game) noexcept {
    ;
  }
};

int main(int argc, char** argv) {
  optimist().execute();

  return 0;
}
