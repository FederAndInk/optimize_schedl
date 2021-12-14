#pragma once

#include "Task.hpp"
#include "neigborhood.hpp"
#include "utils.hpp"

/*
 * 1|2|3|4|5|6
 *      X
 * 1|2|4|3|5|6
 */
inline fai::vector<fai::Index> vnd(fai::vector<Task> const& tasks,
                                   fai::vector<fai::Index>  base_solution)
{
  while (true)
  {
    Consecutive_single_swap_neighborhood n1(base_solution);

    fai::Cost               base_cost = evaluate(tasks, base_solution);
    fai::Cost               best_neigh_cost = base_cost;
    fai::vector<fai::Index> best_neigh;
    for (auto const& neigh_sol : n1)
    {
      if (fai::Cost curr_cost = evaluate(tasks, neigh_sol); //
          best_neigh_cost > curr_cost)
      {
        best_neigh_cost = curr_cost;
        best_neigh = neigh_sol;
      }
    }

    if (base_cost <= best_neigh_cost)
    {
      // change neighborhood
      // if no more neighborhood; end
      return base_solution;
    }
    else
    {
      base_solution = best_neigh;
    }
  }
}

template <typename Select2_fn>
fai::vector<fai::Index> hill_climbing(fai::vector<Task> const& tasks,
                                      fai::vector<fai::Index>  base_solution,
                                      Select2_fn&&             select)
{
  while (true)
  {
    Consecutive_single_swap_neighborhood n1(base_solution);

    fai::Cost               base_cost = evaluate(tasks, base_solution);
    fai::vector<fai::Index> selected_neigh;
    for (auto const& neigh_sol : n1)
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
          selected_neigh = select(tasks, selected_neigh, neigh_sol);
        }
      }
    }

    if (selected_neigh.empty())
    {
      // no more better neighbors
      return base_solution;
    }
    else
    {
      base_solution = selected_neigh;
    }
  }
}

inline fai::vector<fai::Index> const& select2best(fai::vector<Task>              tasks,
                                                  fai::vector<fai::Index> const& lhs,
                                                  fai::vector<fai::Index> const& rhs)
{
  return (evaluate(tasks, lhs) < evaluate(tasks, rhs)) ? lhs : rhs;
}

inline fai::vector<fai::Index> const& select2worst(fai::vector<Task>              tasks,
                                                   fai::vector<fai::Index> const& lhs,
                                                   fai::vector<fai::Index> const& rhs)
{
  return (evaluate(tasks, lhs) < evaluate(tasks, rhs)) ? rhs : lhs;
}