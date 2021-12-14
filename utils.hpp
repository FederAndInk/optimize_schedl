#pragma once

#include <boost/range/adaptor/indexed.hpp>
#include <boost/version.hpp>

#include <iostream>
#include <vector>

// support structured bindings for indexed:  `for (auto [i, elem] : rng | indexed()) {}`
#if BOOST_VERSION < 107400
namespace std
{

#if defined(BOOST_CLANG)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmismatched-tags"
#endif

template <size_t N, class T, class Indexable>
struct tuple_element<N, boost::range::index_value<T, Indexable>> :
  boost::tuples::element<N, boost::range::index_value<T, Indexable>>
{
};

template <class T, class Indexable>
struct tuple_size<boost::range::index_value<T, Indexable>> :
  std::integral_constant<std::size_t, 2>
{
};

#if defined(BOOST_CLANG)
#pragma clang diagnostic pop
#endif

} // namespace std
#endif

namespace fai
{
using Index = int;
}

namespace adp = boost::adaptors;

using namespace std::literals;

template <typename T>
std::ostream& operator<<(std::ostream& out, std::vector<T> const& v)
{
  std::cout << "{";
  if (!v.empty())
  {
    std::copy(std::begin(v), std::end(v) - 1, std::ostream_iterator<T>(std::cout, ", "));
    std::cout << v.back();
  }
  std::cout << "}";
  return out;
}