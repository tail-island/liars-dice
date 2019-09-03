#pragma once

#include <chrono>
#include <future>
#include <string>
#include <vector>

#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include <boost/asio.hpp>
#include <boost/process.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

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
      _cin << parameter << std::endl;  // TODO: チューニング！　バッファーが溢れるみたいで、通信相手がJavaだとやたらと遅い……。

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

      const auto& result = future.get();

      if (result == "") {
        std::cout << "*** COMMUNICATION ERROR on " << _program_path_string << " ***" << std::endl;

        throw std::exception();  // TODO: 専用の例外クラスを作る！
      }

      return result;
    }

    auto check_other_programs(const std::vector<career>& careers) {
      call_program("check_other_programs", write_json(careers, std::function(write_careers)), 40000);
    }

    auto action(const game& game) {
      return read_json(call_program("action", write_json(game, std::function(write_game)), 2000), std::function(read_action));
    }

    auto game_end(const game& game) {
      call_program("game_end", write_json(game, std::function(write_game)), 2000);
    }

    auto terminate() {
      _cin.pipe().close();

      // if (!_child.wait_for(std::chrono::milliseconds(500))) {  // Ubuntu19.04 + boost 1.67だと、必ず500msec待った挙げ句にfalseを返しやがる……。この3行をコメントアウトしてください。
      //   _child.terminate();
      // }
    }

    std::string cerr() noexcept {
      _io_service.run();

      return  _cerr.get();
    }
  };
}
