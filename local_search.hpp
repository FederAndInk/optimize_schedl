#pragma once

#include "Task.hpp"
#include "neighborhood.hpp"
#include "utils.hpp"

#include <fmt/core.h>

#include <chrono>

struct Select2_ret
{
  static constexpr bool BREAK = true;
  static constexpr bool CONTINUE = false;

  // Scheduling const& selected;
  bool brk{false};
};

using Select2_fn_t = Select2_ret (*)(fai::vector<Task> const& tasks,
                                     Scheduling&              lhs,
                                     Scheduling const&        rhs,
                                     fai::Index               imp_neigh_no);

template <typename Neigh_op, typename Select2_fn>
Scheduling next_neighbor(fai::vector<Task> const& tasks,
                         Neigh_op&                neigh_op,
                         Select2_fn&&             select)
{
  fai::Cost base_cost = evaluate(tasks, neigh_op.get_base_solution());

  Scheduling selected_neigh;
  long       nb_neigh = 0;
  fai::Index nb_imp_neigh = 0;
  for (auto const& neigh_sol : neigh_op)
  {
    if (fai::Cost curr_cost = evaluate(tasks, neigh_sol); //
        curr_cost < base_cost)
    {
      if (selected_neigh.empty())
      {
        selected_neigh = neigh_sol;
      }
      else
      {
        Select2_ret select_ret = select(tasks, selected_neigh, neigh_sol, nb_imp_neigh);
        if (select_ret.brk)
        {
          break;
        }
        ++nb_imp_neigh;
      }
    }
    ++nb_neigh;
  }
  fmt::print(" -> treated {} neighbors\n", nb_neigh);
  return selected_neigh;
}

template <typename Select2_fn>
inline Scheduling vnd(fai::vector<Task> const& tasks,
                      Scheduling               base_solution,
                      Select2_fn&&             select)
{
  while (true)
  {
    Consecutive_single_swap_neighborhood n1(base_solution);

    fai::Cost  base_cost = evaluate(tasks, base_solution);
    Scheduling selected_neigh = next_neighbor(tasks, n1, select);

    if (selected_neigh.empty())
    {
      // change neighborhood
      // if no more neighborhood; end
      return base_solution;
    }
    else
    {
      base_solution = selected_neigh;
    }
  }
}

template <typename Neighborhood, typename Select2_fn>
Scheduling hill_climbing(fai::vector<Task> const& tasks,
                         Scheduling               base_solution,
                         Select2_fn&&             select)
{
  fmt::print("hill_climbing with {}\n", get_neighborhood_name<Neighborhood>());
  long nb_loop = 0;
  auto start_time = std::chrono::steady_clock::now();
  while (true)
  {
    fai::Cost    base_cost = evaluate(tasks, base_solution);
    Neighborhood n1(std::move(base_solution));

    std::chrono::duration<double> time_since_start =
      std::chrono::steady_clock::now() - start_time;
    fmt::print("hc_{}: Solution is at {:L} {:.2f} loop/s",
               get_neighborhood_short_name<Neighborhood>(),
               base_cost,
               nb_loop / time_since_start.count());
    Scheduling selected_neigh = next_neighbor(tasks, n1, select);

    if (selected_neigh.empty())
    {
      // no more better neighbors
      return n1.get_base_solution();
    }
    else if (fai::stop_request())
    {
      fmt::print("\nStopped at {} with:\n  {}\n",
                 evaluate(tasks, selected_neigh),
                 selected_neigh);
      return selected_neigh;
    }
    else
    {
      base_solution = std::move(selected_neigh);
    }
    ++nb_loop;
    // if (nb_loop >= 500)
    // {
    //   std::chrono::duration<double> time_since_start =
    //     std::chrono::steady_clock::now() - start_time;
    //   fmt::print("reset after: {:.2f}s\n", time_since_start.count());
    //   nb_loop = 0;
    //   start_time = std::chrono::steady_clock::now();
    // }
  }
}

inline Select2_ret select2best(fai::vector<Task> const&    tasks,
                               Scheduling&                 lhs,
                               Scheduling const&           rhs,
                               [[maybe_unused]] fai::Index imp_neigh_no)
{
  if (evaluate(tasks, lhs) > evaluate(tasks, rhs))
  {
    lhs = rhs;
  }
  return {};
}

inline Select2_ret select2worst(fai::vector<Task> const&    tasks,
                                Scheduling&                 lhs,
                                Scheduling const&           rhs,
                                [[maybe_unused]] fai::Index imp_neigh_no)
{
  if (evaluate(tasks, lhs) < evaluate(tasks, rhs))
  {
    lhs = rhs;
  }
  return {};
}

inline Select2_ret select2first(fai::vector<Task> const&    tasks,
                                Scheduling&                 lhs,
                                Scheduling const&           rhs,
                                [[maybe_unused]] fai::Index imp_neigh_no)
{
  return {Select2_ret::BREAK};
}

template <fai::Index n>
struct select2best_nfirst
{
  constexpr fai::Index get_n() const noexcept
  {
    return n;
  }

  inline Select2_ret operator()(fai::vector<Task> const&    tasks,
                                Scheduling&                 lhs,
                                Scheduling const&           rhs,
                                [[maybe_unused]] fai::Index imp_neigh_no) const
  {
    if (evaluate(tasks, lhs) > evaluate(tasks, rhs))
    {
      lhs = rhs;
    }

    return {imp_neigh_no < n ? Select2_ret::CONTINUE : Select2_ret::BREAK};
  }
};

template <typename U>
struct is_select2best_nfirst
{
private:
  template <typename T>
  static constexpr std::false_type test(T);

  template <fai::Index n>
  static constexpr std::true_type test(select2best_nfirst<n>);

public:
  static constexpr bool value = decltype(test(std::declval<U>()))::value;
};

template <typename Select_fn>
std::string select_fn_name(Select_fn&& fn)
{
  if (fn == select2best)
  {
    return "sbest";
  }
  else if (fn == select2first)
  {
    return "sfirst";
  }
  else if (fn == select2worst)
  {
    return "sworst";
  }
  else
  {
    return "sunknown";
  }
}

template <fai::Index n>
std::string select_fn_name(select2best_nfirst<n>& fn)
{
  return fmt::format("sbestIn{}", n);
}
