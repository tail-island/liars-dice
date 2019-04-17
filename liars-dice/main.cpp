#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#pragma warning(push, 0)
#include <boost/filesystem.hpp>  // Ubuntu18.04のGCCだとfilesystemはexperimentalだったので、boost版でいきます。
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/iterator_range.hpp>
#pragma warning(pop)

#include "dealer.hpp"
#include "util.hpp"

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "usage: liars-dice min-set-count-per-player" << std::endl;
    std::exit(1);
  }

  const auto& min_set_count = std::stoi(argv[1]);

  const auto& program_path_strings = []() {
    auto result = boost::copy_range<std::vector<std::string>>(
      boost::filesystem::directory_iterator(".") |
      boost::adaptors::filtered([](const auto& directory_entry) { return directory_entry.status().type() == boost::filesystem::directory_file; }) |
      boost::adaptors::transformed([](const auto& directory_entry) { auto path = directory_entry.path(); path /= "run.bat"; return path; }) |
      boost::adaptors::filtered([](const auto& path) { return boost::filesystem::exists(path); }) |
      boost::adaptors::transformed([](const auto& path) { return path.string(); }));

    boost::sort(result);

    return result;
  }();

  liars_dice::play_championship(program_path_strings, min_set_count);

  return 0;
}
