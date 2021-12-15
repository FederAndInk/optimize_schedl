#pragma once

#include <boost/range/adaptor/indexed.hpp>
#include <boost/version.hpp>

#include <algorithm>
#include <iostream>
#include <memory>
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
using namespace std::literals;

using Index = int;

template <typename Rng>
Index ssize(Rng&& rng)
{
  return static_cast<Index>(std::size(rng));
}

template <typename T>
class vector : public std::vector<T>
{
public:
  using typename std::vector<T>::value_type;
  using typename std::vector<T>::reference;
  using typename std::vector<T>::const_reference;
  using size_type = fai::Index;
  using v_size_type = typename std::vector<T>::size_type;

  using std::vector<T>::vector;

  explicit vector(size_type n) : std::vector<T>(static_cast<v_size_type>(n)) {}

  reference operator[](size_type n) noexcept
  {
    return std::vector<T>::operator[](static_cast<v_size_type>(n));
  }

  const_reference operator[](size_type n) const noexcept
  {
    return std::vector<T>::operator[](static_cast<v_size_type>(n));
  }

  reference at(size_type n) noexcept
  {
    return std::vector<T>::at(static_cast<v_size_type>(n));
  }

  const_reference at(size_type n) const noexcept
  {
    return std::vector<T>::at(static_cast<v_size_type>(n));
  }

  [[nodiscard]] size_type size() const noexcept
  {
    return static_cast<size_type>(std::vector<T>::size());
  }

  void resize(size_type new_size)
  {
    std::vector<T>::resize(static_cast<v_size_type>(new_size));
  }

  void resize(size_type new_size, const value_type& x)
  {
    std::vector<T>::resize(static_cast<v_size_type>(new_size), x);
  }

  void reserve(size_type n)
  {
    std::vector<T>::reserve(static_cast<v_size_type>(n));
  }
};
} // namespace fai

namespace adp = boost::adaptors;
