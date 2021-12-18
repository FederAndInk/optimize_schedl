#include "../neighborhood.hpp"

#include <boost/range/combine.hpp>

#include <fmt/core.h>
#include <fmt/ranges.h>

#include <algorithm>
#include <cassert>
#include <numeric>
#include <string_view>
#include <typeinfo>

namespace bst = boost;

template <typename Neighborhood>
std::string_view get_neighborhood_name()
{
  if constexpr (std::is_same_v<Neighborhood, Consecutive_single_swap_neighborhood>)
  {
    return "Consecutive_single_swap_neighborhood";
  }
  else if (std::is_same_v<Neighborhood, Reverse_consecutive_single_swap_neighborhood>)
  {
    return "Reverse_consecutive_single_swap_neighborhood";
  }
  else if (std::is_same_v<Neighborhood, Reverse_neighborhood>)
  {
    return "Reverse_neighborhood";
  }
  else
  {
    return "unknown";
  }
}

template <typename T>
std::string highlight(T&& t)
{
  return fmt::format("\033[1;4m{}\033[0m", t);
}

template <typename R1, typename R2>
void print_rng_diffs(R1&& r1, R2&& r2)
{
  for (auto&& [e1, e2] : bst::combine(r1, r2))
  {
    if (e1 != e2)
    {
      fmt::print("{} ", highlight(e2));
    }
    else
    {
      fmt::print("{} ", e2);
    }
  }
}

template <typename Neighborhood>
void test_neighborhood(fai::vector<fai::Index> const& base_sol)
{
  Neighborhood nbh{base_sol};
  fmt::print("{}:\n", get_neighborhood_name<Neighborhood>());

  long nb = 0;
  for (auto&& neigh : nbh)
  {
    print_rng_diffs(base_sol, neigh);
    fmt::print("\n");
    ++nb;
    assert(base_sol != neigh);
  }

  fmt::print("size: {}\n", nbh.size());
  fmt::print("nb: {}\n\n", nb);
  assert(nbh.size() == nb);
}

int main(int argc, char** argv)
{
  fai::vector<fai::Index> base_sol(10);
  std::iota(std::begin(base_sol), std::end(base_sol), 0);
  fmt::print("base: {}\n", base_sol);

  test_neighborhood<Consecutive_single_swap_neighborhood>(base_sol);
  test_neighborhood<Reverse_consecutive_single_swap_neighborhood>(base_sol);
  test_neighborhood<Reverse_neighborhood>(base_sol);
}