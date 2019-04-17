#pragma once

#include <tuple>

#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include <boost/range/combine.hpp>
#include <boost/tuple/tuple.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace util {
  // boost::tupleへの対策。C++17のstructured bindingsと相性が悪くて泣きそう……。

  template <typename BoostTuple, std::size_t... Is>
  auto as_std_tuple(BoostTuple&& boost_tuple, std::index_sequence<Is...>) {
    return std::tuple<typename boost::tuples::element<Is, std::decay_t<BoostTuple>>::type...>(boost::get<Is>(std::forward<BoostTuple>(boost_tuple))...);
  }

  template <typename BoostTuple>
  auto as_std_tuple(BoostTuple&& boost_tuple) {
    return as_std_tuple(std::forward<BoostTuple>(boost_tuple), std::make_index_sequence<boost::tuples::length<std::decay_t<BoostTuple>>::value>());
  }

  template <typename... Ranges>
  auto combine(Ranges&&... ranges) noexcept {
    return boost::combine(ranges...) | boost::adaptors::transformed([](const auto& combined) { return as_std_tuple(combined); });
  }
}
