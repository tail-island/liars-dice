#pragma once

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <random>
#include <unordered_map>
#include <vector>

#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include <boost/algorithm/cxx11/any_of.hpp>
#include <boost/filesystem.hpp>
#include <boost/process.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/irange.hpp>
#include <boost/range/numeric.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "game.hpp"
#include "program_proxy.hpp"
#include "util.hpp"

namespace liars_dice {
  inline auto program_path_nickname(const std::string& program_path_string) noexcept {
    const auto& path  = boost::filesystem::path(program_path_string);
    const auto& paths = std::vector<boost::filesystem::path>(std::begin(path), std::end(path));

    return paths[std::size(paths) - 2].string().substr(0, 7);
  }

  inline auto show_game(const std::vector<std::string>& program_path_strings, const game& game, const std::vector<int>& dice_count_deltas) noexcept {
    std::cout << "# Dices" << std::endl;
    std::cout << std::endl;

    for (const auto& [program_path_string, player]: util::combine(program_path_strings, game.players())) {
      std::cout << program_path_nickname(program_path_string) << "\t";
      for (const auto& face: player.faces()) {
        std::cout << face << " ";
      }
      std::cout << std::endl;
    }

    std::cout << std::endl;
    std::cout << "# Counts" << std::endl;
    std::cout << std::endl;

    for (const auto& face: boost::irange(2, 7)) {
      std::cout << face << " = " << game.face_count(face) << std::endl;
    }

    std::cout << std::endl;
    std::cout << "# Actions" << std::endl;
    std::cout << std::endl;

    [&]() {
      for (auto i = 0; ; ++i) {
        for (auto j = 0; j < static_cast<int>(std::size(game.players())); ++j) {
          if (i >= static_cast<int>(std::size(game.players()[j].actions()))) {
            return;
          }

          const auto& action = game.players()[j].actions()[i];

          if (action.bid()) {
            std::cout << program_path_nickname(program_path_strings[j]) << "\t" << action.bid().value().face() << " " << action.bid().value().min_count() << "'s." << std::endl;
          }

          if (action.challenge()) {
            std::cout << program_path_nickname(program_path_strings[j]) << "\t" << "challenge." << std::endl;
          }
        }
      }
    }();

    std::cout << std::endl;
    std::cout << "# Results" << std::endl;
    std::cout << std::endl;

    for (const auto& [program_path_string, dice_count_delta]: util::combine(program_path_strings, dice_count_deltas)) {
      std::cout << program_path_nickname(program_path_string) << "\t" << dice_count_delta << std::endl;
    }

    std::cout << std::endl;
  }

  inline auto show_scores(const std::vector<std::string>& program_path_strings, const std::vector<float>& scores) noexcept {
    std::cout << "# Scores" << std::endl;
    std::cout << std::endl;

    auto program_path_string_and_scores = boost::copy_range<std::vector<std::tuple<std::string, float>>>(util::combine(program_path_strings, scores));

    boost::sort(program_path_string_and_scores, [](const auto& program_path_string_and_score_1, const auto& program_path_string_and_score_2) { return std::get<1>(program_path_string_and_score_1) < std::get<1>(program_path_string_and_score_2); });
    boost::reverse(program_path_string_and_scores);

    for (const auto& [program_path_string, score]: program_path_string_and_scores) {
      std::cout << program_path_nickname(program_path_string) << "\t" << std::fixed << std::setprecision(3) << score << std::defaultfloat << std::endl;
    }

    std::cout << std::endl;
  }

  inline auto show_logs(const std::vector<std::string>& program_path_strings, const std::vector<std::string>& log_strings) noexcept {
    std::cout << "# Logs" << std::endl;
    std::cout << std::endl;

    for (const auto& [program_path_string, log_string]: util::combine(program_path_strings, log_strings)) {
      auto stream = std::stringstream(log_string);

      for (auto line = std::string(); std::getline(stream, line); ) {
        std::cout << program_path_nickname(program_path_string) << "\t" << line << std::endl;
      }
    }

    std::cout << std::endl;
  }

  inline auto play_championship(const std::vector<std::string>& program_path_strings, int min_set_count) noexcept {
    using program_path_t = std::string;
    using program_id_t   = std::string;

    auto past_games = std::vector<std::tuple<std::unordered_map<program_path_t, program_id_t>, game>>();

    // 他のプログラムの性格診断向けのデータを作成する関数。
    const auto& careers = [&](const auto& program_paths, const auto& program_ids) {
      auto result = std::vector<career>(); result.reserve(std::size(program_paths));

      for (auto i = 0; i < static_cast<int>(std::size(program_paths)); ++i) {
        auto career_records = std::vector<career_record>(); career_records.reserve(100);

        for (auto j = static_cast<int>(std::size(past_games)) - 1; j >= 0 && std::size(career_records) < 100; --j) {
          const auto& it = std::get<0>(past_games[j]).find(program_paths[i]);
          if (it == std::end(std::get<0>(past_games[j]))) {
            continue;
          }
          career_records.emplace_back(career_record{it->second, std::get<1>(past_games[j])});
        }

        result.emplace_back(career{program_ids[i], career_records});
      }

      return result;

      // 重たそうな処理だったので、敢えて手続き型で書いてみました。
    };

    // 最後の一人になるまでゲームを繰り返す関数。
    const auto& play_set = [&](const auto& program_paths) {
      auto random_engine = std::mt19937_64(std::random_device()());

      // プログラム側からの追跡を困難にするために、セット毎にプログラムにIDを振り直します。
      const auto& program_ids = boost::copy_range<std::unordered_map<program_path_t, program_id_t>>(
        program_paths |
        boost::adaptors::indexed() |
        boost::adaptors::transformed([](const auto& indexed_program_path) { return std::make_pair(indexed_program_path.value(), std::string(1, 'A' + indexed_program_path.index())); }));

      // プログラム毎のダイスの数。
      auto program_dice_counts = boost::copy_range<std::unordered_map<program_path_t, int>>(
        program_paths |
        boost::adaptors::transformed([](const auto& program_path) { return std::make_pair(program_path, 5); }));

      // プログラムのプロキシー。
      auto program_proxies = boost::copy_range<std::unordered_map<program_path_t, std::shared_ptr<program_proxy>>>(
        program_paths |
        boost::adaptors::transformed([](const auto& program_path) { return std::make_pair(program_path, std::make_shared<program_proxy>(program_path)); }));

      // 敗退者リスト。一度に複数人退場することがあるので、配列の配列にしました。
      auto losers_collection = std::vector<std::vector<program_path_t>>();

      // 他のプログラムの戦歴をプログラムに通知します。
      [&]() {
        const auto& careers_ = [&]() {
          const auto& program_ids_ = boost::copy_range<std::vector<program_id_t>>(
            program_paths |
            boost::adaptors::transformed([&](const auto& program_path) { return program_ids.at(program_path); }));

          return careers(program_paths, program_ids_);
        }();

        for (const auto& program_path: program_paths) {
          try {
            program_proxies.at(program_path)->check_other_programs(careers_);

          } catch (...) {
            program_dice_counts[program_path] = 0;

            losers_collection.emplace_back(std::vector<program_path_t>{program_path});
          }
        }
      }();

      // 最後の一人になるまでゲームを繰り返します。
      while (boost::count_if(program_dice_counts, [&](const auto& program_dice_count) { return program_dice_count.second > 0; }) > 1) {
        // 今回のゲームに参加する、まだダイスが残っているプログラムを抽出します。ついでなので、ここで席順もシャッフルしておきます。
        const auto& in_game_program_paths = [&]() {
          auto result = boost::copy_range<std::vector<program_path_t>>(
            program_paths |
            boost::adaptors::filtered([&](const auto& program_path) { return program_dice_counts.at(program_path) > 0; }));

          std::shuffle(std::begin(result), std::end(result), random_engine);

          return result;
        }();

        // ゲームを実行します。
        const auto& [game, dice_count_deltas] = [&]() {
          const auto& ids = boost::copy_range<std::vector<program_id_t>>(
            in_game_program_paths |
            boost::adaptors::transformed([&](const auto& in_game_program_path) { return program_ids.at(in_game_program_path); }));

          const auto& dice_counts = boost::copy_range<std::vector<int>>(
            in_game_program_paths |
            boost::adaptors::transformed([&](const auto& in_game_program_path) { return program_dice_counts.at(in_game_program_path); }));

          const auto& action_functions = boost::copy_range<std::vector<std::function<action(const liars_dice::game&)>>>(
            in_game_program_paths |
            boost::adaptors::transformed([&](const auto& in_game_program_path) { return [&](const auto& game) { return program_proxies.at(in_game_program_path)->action(game); }; }));

          return play_game(ids, dice_counts, action_functions);
        }();

        // ゲームの内容を表示します。
        show_game(in_game_program_paths, game, dice_count_deltas);

        // ゲーム終了をプログラムに通知します。昨年の「ごろごろどうぶつしょうぎ」では、この通知を入れ忘れて参加者に不便を強いてしまいました……。
        for (const auto& in_game_program_path: in_game_program_paths) {
          try {
            program_proxies.at(in_game_program_path)->game_end(game);

          } catch (...) {
            if (program_dice_counts.at(in_game_program_path) > 0) {
              program_dice_counts[in_game_program_path] = 0;
            }
          }
        }

        // ゲームを、過去のゲーム集に追加します。
        [&, game = game]() {  // P0588R1...
          const auto& game_program_ids = boost::copy_range<std::unordered_map<program_path_t, program_id_t>>(
            in_game_program_paths |
            boost::adaptors::transformed([&](const auto& in_game_program_path) { return std::make_pair(in_game_program_path, program_ids.at(in_game_program_path)); }));

          past_games.emplace_back(game_program_ids, game);
        }();

        // プログラムのダイスを減らします。
        for (const auto& [in_game_program_path, dice_count_delta]: util::combine(in_game_program_paths, dice_count_deltas)) {
          program_dice_counts.at(in_game_program_path) += dice_count_delta;
        }

        // 敗退者リストをメンテナンスします。
        [&]() {
          auto losers = boost::copy_range<std::vector<program_path_t>>(
            in_game_program_paths |
            boost::adaptors::filtered([&](const auto& in_game_program_path) { return program_dice_counts.at(in_game_program_path) <= 0; }));

          if (!std::empty(losers)) {
            boost::for_each(
              boost::irange(0, static_cast<int>(std::size(losers)) - 1),
              [&](const auto& _) {
                losers_collection.emplace_back(std::vector<program_path_t>());
              });

            losers_collection.emplace_back(losers);
          }
        }();
      }

      // プログラムを終了させます。
      for (const auto& program_path: program_paths) {
        try {
          program_proxies.at(program_path)->terminate();

        } catch (...) {
          ;
        }
      }

      // 標準エラー出力を出力します。
      [&]() {
        const auto& program_logs = boost::copy_range<std::vector<std::string>>(
          program_paths |
          boost::adaptors::transformed([&](const auto& program_path) { return program_proxies.at(program_path)->cerr(); }));

        show_logs(program_paths, program_logs);
      }();

      // 最後まで生き残ったプログラムを、最後の敗退者として登録します。
      losers_collection.emplace_back(std::vector<program_path_t>{boost::find_if(program_dice_counts, [&](const auto& program_dice_count) { return program_dice_count.second > 0; })->first});

      // スコアを計算して返します。
      return boost::copy_range<std::vector<float>>(
        program_paths |
        boost::adaptors::transformed(
          [&](const auto& program_path) {
            for (auto i = 5; i >= 0; --i) {
              if (boost::find(losers_collection[i], program_path) != std::end(losers_collection[i])) {
                return static_cast<float>(boost::accumulate(boost::irange(i, i - static_cast<int>(std::size(losers_collection[i])), -1), 0)) / std::size(losers_collection[i]);
              }
            }

            std::cout << "*** WHY THIS EXCEPTION? ***" << std::endl;
            throw std::exception();
          }));
    };

    // すべてのプログラムがmin_set_countで指定した回数以上のセットを実行するまでセットを繰り返す関数。
    const auto& play_sets = [&](const auto& program_paths) {
      class program_evaluation final {
        float _total_score;
        int _set_count;

      public:
        program_evaluation() noexcept: _total_score(0.0f), _set_count(0) {
          ;
        }

        const auto& set_count() const noexcept {
          return _set_count;
        }

        auto value() const noexcept {
          return _total_score / _set_count;
        }

        auto add_score(float score) noexcept {
          _total_score += score;

          _set_count++;
        }
      };

      auto random_engine = std::mt19937_64(std::random_device()());

      auto program_evaluations = boost::copy_range<std::unordered_map<program_path_t, program_evaluation>>(
        program_paths |
        boost::adaptors::transformed([](const auto& program_path) { return std::make_pair(program_path, program_evaluation()); }));

      while (boost::algorithm::any_of(program_evaluations, [&](const auto& program_evaluation) { return program_evaluation.second.set_count() < min_set_count; })) {
        const auto& sampled_program_paths = [&]() {
          auto result = std::vector<std::string>();

          std::sample(std::begin(program_paths), std::end(program_paths), std::back_inserter(result), 6, random_engine);
          std::shuffle(std::begin(result), std::end(result), random_engine);

          return result;
        }();

        const auto& scores = play_set(sampled_program_paths);

        for (const auto& [program_path, score]: util::combine(sampled_program_paths, scores)) {
          program_evaluations.at(program_path).add_score(score);
        }

        [&]() {
          const auto& scores = boost::copy_range<std::vector<float>>(
            program_paths |
            boost::adaptors::transformed([&](const auto& program_path) { return program_evaluations.at(program_path).value(); }));

          show_scores(program_paths, scores);
        }();
      }

      return boost::copy_range<std::vector<float>>(
        program_paths |
        boost::adaptors::transformed([&](const auto& program_path) { return program_evaluations.at(program_path).value(); }));
    };

    const auto& result = play_sets(program_path_strings);

    // あとで何かに使えるかもしれないので、全ての試合を記録しておきます。
    [&]() {
      auto ofstream = std::ofstream("all-games.json");
      ofstream << write_json(past_games, std::function(write_past_games));
      ofstream.close();
    }();

    return result;
  }
}
