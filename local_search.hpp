#pragma once

#include "Task.hpp"
#include "neigborhood.hpp"

#include <vector>

/*
 * 1|2|3|4|5|6
 *      X
 * 1|2|4|3|5|6
 */
inline std::vector<fai::Index> vnd(std::vector<Task>       tasks,
                                   std::vector<fai::Index> base_solution)
{
  while (true)
  {
    Consecutive_single_swap_neighborhood n1(base_solution);

    fai::Cost               base_cost = evaluate(tasks, base_solution);
    fai::Cost               best_neigh_cost = base_cost;
    std::vector<fai::Index> best_neigh;
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

inline std::vector<fai::Index> hill_climbing(std::vector<Task>       tasks,
                                             std::vector<fai::Index> base_solution)
{
  return {};
}