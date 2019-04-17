#pragma once

#include <string>
#include <vector>

#include "game.hpp"
#include "json.hpp"

namespace liars_dice {
  class program {
  public:
    virtual void check_other_programs(const std::vector<career>& careers) noexcept = 0;
    virtual liars_dice::action action(const game& game) noexcept = 0;
    virtual void game_end(const game& game) noexcept = 0;
    virtual void terminate() noexcept {
      ;
    }

    auto execute() {
      for (auto command_string = std::string(); std::getline(std::cin, command_string); ) {
        auto parameter_string = std::string(); std::getline(std::cin, parameter_string);

        if (command_string == "check_other_programs") {
          check_other_programs(read_json(parameter_string, std::function(read_careers))); std::cout << "OK" << std::endl;

          continue;
        }

        if (command_string == "action") {
          std::cout << write_json(action(read_json(parameter_string, std::function(read_game))), std::function(write_action)) << std::endl;

          continue;
        }

        if (command_string == "game_end") {
          game_end(read_json(parameter_string, std::function(read_game))); std::cout << "OK" << std::endl;

          continue;
        }

        if (command_string == "terminate") {
          terminate();

          break;
        }
      }
    }
  };
}
