#include "Task.hpp"
#include "heuristics.hpp"
#include "local_search.hpp"
#include "utils.hpp"

#include "fmt/core.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <locale>
#include <numeric>
#include <random>
#include <stdexcept>

fai::vector<fai::Index> read_solution(std::istream& in, fai::Index nb_tasks)
{
  fai::vector<fai::Index> sol;
  sol.reserve(nb_tasks);
  std::copy_n(std::istream_iterator<fai::Index>(in), nb_tasks, std::back_inserter(sol));
  return sol;
}

fai::vector<Task> read_tasks(std::istream& in)
{
  int nb_task;
  in >> nb_task;
  fai::vector<Task> tasks;
  tasks.reserve(nb_task);
  for (int i = 0; i < nb_task; ++i)
  {
    in >> tasks.emplace_back();
    tasks.back().no = i;
  }
  return tasks;
}

fai::vector<fai::Index> generate_random_solution(fai::Index nb_tasks)
{
  fai::vector<fai::Index> sol(nb_tasks);
  std::iota(sol.begin(), sol.end(), 0);
  std::shuffle(sol.begin(), sol.end(), std::mt19937{std::random_device{}()});
  return sol;
}

int main(int argc, char** argv)
{
  if (argc < 2)
  {
    std::cout << "usage: " << argv[0] << " tasks_file [scheduling_file]\n";
    return 1;
  }
  std::ifstream tasks_file(argv[1]);
  if (!tasks_file)
  {
    fmt::print("Error opening file {}", argv[1]);
    return 1;
  }
  fai::vector<Task>       tasks = read_tasks(tasks_file);
  fai::vector<fai::Index> sol;
  if (argc == 3)
  {
    std::ifstream sol_file(argv[2]);
    sol = read_solution(sol_file, tasks.size());
  }
  else
  {
    sol = generate_random_solution(tasks.size());
  }
  std::cout << "Random Scheduling: " << sol << "\n";

  try
  {
    std::locale::global(std::locale{"en_US.UTF-8"});
  }
  catch (std::runtime_error const& e)
  {
    std::cout << "Locale en_US.UTF-8 not on your system, falling back to C\n";
    std::locale::global(std::locale{"C"});
  }
  fmt::print("Total cost: {:L}\n", evaluate(tasks, sol));

  for (auto&& heuristic : get_heuristics())
  {

    sol = ct_heuristic(tasks, select(heuristic.fn));
    fmt::print("Total cost {} heuristic: {:L}\n", heuristic.name, evaluate(tasks, sol));
  }

  sol = ct_heuristic(tasks, select(eval_sdelay_div_weight));
  fmt::print("Total cost eval_sdelay_div_weight heuristic: {:L}\n", evaluate(tasks, sol));

  sol = ct_heuristic(tasks, select(eval_static_sdelay_div_weight));
  fmt::print("Total cost eval_static_sdelay_div_weight heuristic: {:L}\n",
             evaluate(tasks, sol));

  sol = vnd(tasks, sol);
  fmt::print("Total cost vnd from eval_static_sdelay_div_weight: {:L}\n",
             evaluate(tasks, sol));

  sol = ct_heuristic(tasks, select(eval_static_sdelay_mul_weight));
  fmt::print("Total cost eval_static_sdelay_mul_weight heuristic: {:L}\n",
             evaluate(tasks, sol));

  sol = ct_heuristic(tasks, select(eval_static_sdelay));
  fmt::print("Total cost eval_static_sdelay heuristic: {:L}\n", evaluate(tasks, sol));

  sol = ct_heuristic(tasks, select(eval_static_expiry));
  fmt::print("Total cost eval_static_expiry heuristic: {:L}\n", evaluate(tasks, sol));

  sol = ct_heuristic(tasks, select(eval_static_expiry_div_weight_mul_time));
  fmt::print("Total cost eval_static_expiry_div_weight_mul_time heuristic: {:L}\n",
             evaluate(tasks, sol));
}
