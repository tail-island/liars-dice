﻿#pragma once

#include <chrono>
#include <future>
#include <string>
#include <vector>

#pragma warning(push, 0)
#include <boost/asio.hpp>
#include <boost/process.hpp>
#pragma warning(pop)

#include "game.hpp"
#include "json.hpp"

namespace liars_dice {
  class program_proxy final {
    std::string _program_path_string;

    boost::process::opstream _cin;
    boost::process::ipstream _cout;
    std::future<std::string> _cerr;

    boost::asio::io_service _io_service;

    boost::process::child _child;

  public:
    program_proxy(const std::string& program_path_string) noexcept:
      _program_path_string(program_path_string),
      _child(_program_path_string, boost::process::std_in < _cin, boost::process::std_out > _cout, boost::process::std_err > _cerr, _io_service)
    {
      ;
    }

    auto call_program(const std::string& command, const std::string& parameter, int timeout_milliseconds) {
      if (!_child.running()) {
        std::cout << "*** COMMUNICATION ERROR on " << _program_path_string << " ***" << std::endl;

        throw std::exception();  // TODO: 専用の例外クラスを作る！
      }

      _cin << command   << std::endl;
      _cin << parameter << std::endl;

      auto future = std::async(
        std::launch::async,
        [&]() {
          auto result = std::string();

          std::getline(_cout, result);

          return result;
        });

      if (future.wait_for(std::chrono::milliseconds(timeout_milliseconds)) == std::future_status::timeout) {
        std::cout << "*** TIMEOUT on " << _program_path_string << " ***" << std::endl;

        throw std::exception();  // TODO: 専用の例外クラスを作る！
      }

      return future.get();
    }

    // auto check_other_programs(const std::vector<career>& careers) {
    //   call_program("check_other_programs", write_json(careers, std::function(write_careers)), 5000);
    // }

    auto action(const game& game) {
      return read_json(call_program("action", write_json(game, std::function(write_game)), 500), std::function(read_action));
    }

    auto game_end(const game& game) {
      call_program("game_end", write_json(game, std::function(write_game)), 500);
    }

    auto terminate() {
      call_program("terminate", "", 500);

      if (!_child.wait_for(std::chrono::milliseconds(1000))) {
        _child.terminate();
      }
    }

    std::string cerr() noexcept {
      _io_service.run();

      return  _cerr.get();
    }
  };
}