#include "../neighborhood.hpp"
#include "../utils.hpp"

#include <boost/range/adaptors.hpp>
#include <boost/range/combine.hpp>

#include <fmt/core.h>
#include <fmt/ranges.h>

#include <algorithm>
#include <numeric>
#include <random>
#include <string_view>
#include <typeinfo>

namespace bst = boost;
namespace adp = boost::adaptors;

long failed_test = 0;

bool assert_equal_fn(bool             expr,
                     std::string_view msg,
                     std::string_view expr_str,
                     std::size_t      line,
                     std::string_view file)
{
  if (!expr)
  {
    ++failed_test;
    fmt::print(stderr,
               "{}:{}: \033[31massert error\033[0m({}), {}\n",
               file,
               line,
               expr_str,
               msg);
  }
  return expr;
}

#define assert_equal(expr, msg) assert_equal_fn((expr), (msg), #expr, __LINE__, __FILE__)

template <typename T>
std::string highlight(T&& t)
{
  return fmt::format("\033[1;4m{}\033[0m", t);
}

template <typename R1, typename R2>
void print_rng_diffs(R1&& r1, R2&& r2)
{
  bool first = true;
  fmt::print("{{");
  for (auto&& [e1, e2] : bst::combine(r1, r2))
  {
    if (!first)
    {
      fmt::print(", ");
    }
    if (e1 != e2)
    {
      fmt::print("{}", highlight(e2));
    }
    else
    {
      fmt::print("{}", e2);
    }
    first = false;
  }
  fmt::print("}}");
}

template <typename Neighborhood, typename Rng>
void test_neighborhood(fai::vector<fai::Index> const& base_sol, Rng const& rng_expected)
{
  Neighborhood nbh{base_sol};
  fmt::print("{}:\n", get_neighborhood_name<decltype(nbh)>());

  long nb = 0;
  auto expect_it = std::begin(rng_expected);
  auto expect_end = std::end(rng_expected);
  for (auto&& neigh : nbh)
  {
    assert_equal(expect_it != expect_end, "reached end to soon");
    print_rng_diffs(base_sol, neigh);
    fmt::print("\n");
    assert_equal(
      neigh == *expect_it,
      fmt::format("neighbor isn't generated correctly\n  | expected {}", *expect_it));
    assert_equal(base_sol != neigh, "base solution shouldn't be in the neighbors");
    ++nb;
    ++expect_it;
  }

  fmt::print("size: {}\n", nbh.size());
  fmt::print("nb: {}\n\n", nb);
  assert_equal(nbh.size() == nb, "size isn't correctly computed");
}

int main(int argc, char** argv)
{
  fai::vector<fai::Index> base_sol(10);
  std::iota(std::begin(base_sol), std::end(base_sol), 0);
  fmt::print("base: {}\n", base_sol);

  std::vector<fai::vector<fai::Index>> cssn_neighs{{1, 0, 2, 3, 4, 5, 6, 7, 8, 9},
                                                   {0, 2, 1, 3, 4, 5, 6, 7, 8, 9},
                                                   {0, 1, 3, 2, 4, 5, 6, 7, 8, 9},
                                                   {0, 1, 2, 4, 3, 5, 6, 7, 8, 9},
                                                   {0, 1, 2, 3, 5, 4, 6, 7, 8, 9},
                                                   {0, 1, 2, 3, 4, 6, 5, 7, 8, 9},
                                                   {0, 1, 2, 3, 4, 5, 7, 6, 8, 9},
                                                   {0, 1, 2, 3, 4, 5, 6, 8, 7, 9},
                                                   {0, 1, 2, 3, 4, 5, 6, 7, 9, 8}};

  test_neighborhood<Consecutive_single_swap_neighborhood>(base_sol, cssn_neighs);
  test_neighborhood<Backward_neighborhood<Consecutive_single_swap_neighborhood>>(
    base_sol,
    cssn_neighs | adp::reversed);
  std::vector<fai::vector<fai::Index>> rn_neighs{
    {1, 0, 2, 3, 4, 5, 6, 7, 8, 9}, {2, 1, 0, 3, 4, 5, 6, 7, 8, 9},
    {3, 2, 1, 0, 4, 5, 6, 7, 8, 9}, {4, 3, 2, 1, 0, 5, 6, 7, 8, 9},
    {5, 4, 3, 2, 1, 0, 6, 7, 8, 9}, {6, 5, 4, 3, 2, 1, 0, 7, 8, 9},
    {7, 6, 5, 4, 3, 2, 1, 0, 8, 9}, {8, 7, 6, 5, 4, 3, 2, 1, 0, 9},
    {9, 8, 7, 6, 5, 4, 3, 2, 1, 0}, {0, 2, 1, 3, 4, 5, 6, 7, 8, 9},
    {0, 3, 2, 1, 4, 5, 6, 7, 8, 9}, {0, 4, 3, 2, 1, 5, 6, 7, 8, 9},
    {0, 5, 4, 3, 2, 1, 6, 7, 8, 9}, {0, 6, 5, 4, 3, 2, 1, 7, 8, 9},
    {0, 7, 6, 5, 4, 3, 2, 1, 8, 9}, {0, 8, 7, 6, 5, 4, 3, 2, 1, 9},
    {0, 9, 8, 7, 6, 5, 4, 3, 2, 1}, {0, 1, 3, 2, 4, 5, 6, 7, 8, 9},
    {0, 1, 4, 3, 2, 5, 6, 7, 8, 9}, {0, 1, 5, 4, 3, 2, 6, 7, 8, 9},
    {0, 1, 6, 5, 4, 3, 2, 7, 8, 9}, {0, 1, 7, 6, 5, 4, 3, 2, 8, 9},
    {0, 1, 8, 7, 6, 5, 4, 3, 2, 9}, {0, 1, 9, 8, 7, 6, 5, 4, 3, 2},
    {0, 1, 2, 4, 3, 5, 6, 7, 8, 9}, {0, 1, 2, 5, 4, 3, 6, 7, 8, 9},
    {0, 1, 2, 6, 5, 4, 3, 7, 8, 9}, {0, 1, 2, 7, 6, 5, 4, 3, 8, 9},
    {0, 1, 2, 8, 7, 6, 5, 4, 3, 9}, {0, 1, 2, 9, 8, 7, 6, 5, 4, 3},
    {0, 1, 2, 3, 5, 4, 6, 7, 8, 9}, {0, 1, 2, 3, 6, 5, 4, 7, 8, 9},
    {0, 1, 2, 3, 7, 6, 5, 4, 8, 9}, {0, 1, 2, 3, 8, 7, 6, 5, 4, 9},
    {0, 1, 2, 3, 9, 8, 7, 6, 5, 4}, {0, 1, 2, 3, 4, 6, 5, 7, 8, 9},
    {0, 1, 2, 3, 4, 7, 6, 5, 8, 9}, {0, 1, 2, 3, 4, 8, 7, 6, 5, 9},
    {0, 1, 2, 3, 4, 9, 8, 7, 6, 5}, {0, 1, 2, 3, 4, 5, 7, 6, 8, 9},
    {0, 1, 2, 3, 4, 5, 8, 7, 6, 9}, {0, 1, 2, 3, 4, 5, 9, 8, 7, 6},
    {0, 1, 2, 3, 4, 5, 6, 8, 7, 9}, {0, 1, 2, 3, 4, 5, 6, 9, 8, 7},
    {0, 1, 2, 3, 4, 5, 6, 7, 9, 8}};
  test_neighborhood<Reverse_neighborhood>(base_sol, rn_neighs);
  test_neighborhood<Backward_neighborhood<Reverse_neighborhood>>(base_sol,
                                                                 rn_neighs |
                                                                   adp::reversed);

  std::random_device rd{};
  std::mt19937       gen(rd());

  Reverse_neighborhood          ntest{base_sol};
  std::uniform_int_distribution dist(0, ntest.size() - 1);

  fai::Index random_neigh_idx = dist(gen);
  auto       rand_pick = ntest.begin();
  rand_pick += random_neigh_idx;
  fmt::print("random access test\n");
  assert_equal(
    *rand_pick == rn_neighs[random_neigh_idx],
    fmt::format("neighbor isn't generated correctly at pos {}", random_neigh_idx));
  fmt::print("  | received ");
  print_rng_diffs(base_sol, *rand_pick);
  fmt::print("\n  | expected ");
  print_rng_diffs(base_sol, rn_neighs[random_neigh_idx]);
  fmt::print("\n");

  if (failed_test != 0)
  {
    fmt::print(stderr, "{} \033[31;1mtests failed\033[0m\n", failed_test);
    return 1;
  }
  else
  {
    fmt::print(stderr, "\033[32;1mAll tests passed\033[0m\n");
  }
}