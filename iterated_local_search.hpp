#include "local_search.hpp"
#include "neighborhood.hpp"

#include <iterator>
#include <random>

template <class Neighborhood>
fai::vector<fai::Index> random_neighbor(fai::vector<fai::Index> base_solution)
{
  static std::mt19937             gen(std::random_device{}());
  Neighborhood                    neighborhood(base_solution);
  std::uniform_int_distribution<> distrib(0, neighborhood.size());
  neighborhood.at(distrib(gen));
};

template <typename Local_search_fn, typename Disturb_fn, typename Accept_fn>
fai::vector<fai::Index> ils(fai::vector<Task> const& tasks,
                            fai::vector<fai::Index>  base_solution,
                            Local_search_fn&&        local_search_fn,
                            Disturb_fn&&             disturb_fn,
                            Accept_fn&&              accept_fn)
{
  std::vector<fai::vector<fai::Index>> history;
  fai::vector<fai::Index> first_opt_sol = local_search_fn(tasks, base_solution);
  do
  {
    fai::vector<fai::Index> second_opt_sol =
      local_search_fn(tasks, disturb_fn(first_opt_sol, history));
    first_opt_sol = accept_fn(first_opt_sol, second_opt_sol, history);
  } while (true /*exit_condition*/);
  return first_opt_sol;
}