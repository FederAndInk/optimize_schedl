#include "Task.hpp"
#include "local_search.hpp"
#include "neighborhood.hpp"
#include "utils.hpp"

#include <iterator>
#include <random>

// disturb function
template <class Neighborhood>
Scheduling random_neighbor(Scheduling const& base_solution)
{
  static std::mt19937             gen(std::random_device{}());
  Neighborhood                    neighborhood(base_solution);
  std::uniform_int_distribution<> distrib(0, neighborhood.size() - 1);
  return neighborhood.at(distrib(gen));
};

// disturb function
template <class Neighborhood>
Scheduling random_distant_neighbor(Scheduling               solution,
                                   fai::Index               distance,
                                   std::vector<Scheduling>& history)
{
  for (fai::Index i = 0; i < distance; ++i)
  {
    solution = random_neighbor<Neighborhood>(solution);
  }
  return solution;
}

// disturb function
template <class Neighborhood>
class Random_dyn_dist_neighbor
{
  double     aug;
  double     step;
  fai::Index max_step;

public:
  Random_dyn_dist_neighbor(double     aug = 1.05,
                           fai::Index step = 10,
                           fai::Index max_step = 100)
    : aug(aug), step(step)
  {
  }

  Scheduling operator()(Scheduling solution) noexcept
  {
    solution =
      random_distant_neighbor<Neighborhood>(solution, static_cast<fai::Index>(step));
    if (step < max_step)
    {
      step *= aug;
    }
    return solution;
  }
};

// accept function
inline void accept_best(fai::vector<Task> const& tasks,
                        Scheduling&              accepted_sol,
                        Scheduling&&             new_sol,
                        std::vector<Scheduling>& history)
{
  history.push_back(std::move(new_sol));
  if (evaluate(tasks, history.back()) < evaluate(tasks, accepted_sol))
  {
    accepted_sol = history.back();
  }
}

// stop function
template <fai::Index n>
bool stop_n_worse(fai::vector<Task> const& tasks, std::vector<Scheduling> history)
{
  auto it = std::min_element(std::rbegin(history),
                             std::rend(history),
                             [&tasks](Scheduling const& lhs, Scheduling const& rhs)
                             { return evaluate(tasks, lhs) <= evaluate(tasks, rhs); });
  fmt::print("stop_{}_worse: dist: {}\n", n, std::distance(std::rbegin(history), it));
  return std::distance(std::rbegin(history), it) >= n;
}

template <typename Local_search_fn,
          typename Disturb_fn,
          typename Accept_fn,
          typename Stop_fn>
Scheduling ils(fai::vector<Task> const& tasks,
               Scheduling               base_solution,
               Local_search_fn&&        local_search_fn,
               Disturb_fn&&             disturb_fn,
               Accept_fn&&              accept_fn,
               Stop_fn&&                stop_fn)
{
  std::vector<Scheduling> history;
  Scheduling              accepted_sol = local_search_fn(tasks, std::move(base_solution));
  history.push_back(accepted_sol);
  do
  {
    Scheduling second_opt_sol = local_search_fn(tasks, disturb_fn(accepted_sol, history));
    accept_fn(tasks, accepted_sol, std::move(second_opt_sol), history);
  } while (!fai::stop_request() && !stop_fn(tasks, history));
  return *std::min_element(std::begin(history),
                           std::end(history),
                           [&tasks](Scheduling const& lhs, Scheduling const& rhs)
                           { return evaluate(tasks, lhs) < evaluate(tasks, rhs); });
}