#include "Task.hpp"
#include "heuristics.hpp"
#include "local_search.hpp"
#include "utils.hpp"

#include <fmt/core.h>
#include <fmt/ranges.h>

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
    fmt::print("usage: {} tasks_file [scheduling_file]\n", argv[0]);
    return 1;
  }
  std::ifstream tasks_file(argv[1]);
  if (!tasks_file)
  {
    fmt::print("Error opening file {}", argv[1]);
    return 1;
  }
  fai::vector<Task>       tasks = read_tasks(tasks_file);
  std::string_view        best_algo;
  fai::vector<fai::Index> sol;
  if (argc == 3)
  {
    std::ifstream sol_file(argv[2]);
    if (!sol_file)
    {
      fmt::print("Error opening file {}", argv[2]);
      return 1;
    }
    sol = read_solution(sol_file, tasks.size());
    best_algo = "user provided";
  }
  else
  {
    sol = generate_random_solution(tasks.size());
    best_algo = "random";
    fmt::print("Random ");
  }
  fmt::print("Scheduling: {}\n", sol);

  try
  {
    std::locale::global(std::locale{"en_US.UTF-8"});
  }
  catch (std::runtime_error const& e)
  {
    fmt::print("Locale en_US.UTF-8 not on your system, falling back to C\n");
    std::locale::global(std::locale{"C"});
  }
  fai::Cost sol_cost = evaluate(tasks, sol);
  fmt::print("Total cost: {:L}\n", sol_cost);

  fai::vector<fai::Index> best_sol = sol;
  fai::Cost               best_sol_cost = sol_cost;
  for (auto&& heuristic : get_heuristics())
  {
    sol = ct_heuristic(tasks, select(heuristic.fn));
    sol_cost = evaluate(tasks, sol);
    fmt::print("Total cost {} heuristic: {:L}\n", heuristic.name, sol_cost);

    if (best_sol_cost > sol_cost)
    {
      best_sol = sol;
      best_algo = heuristic.name;
      best_sol_cost = sol_cost;
    }
  }
  fmt::print("\nBest algo: {} with cost: {:L}\n", best_algo, best_sol_cost);

  // auto best_vnd_sol = vnd(tasks, best_sol);
  // fmt::print("Total cost vnd from {}: {:L}\n", best_algo, evaluate(tasks,
  // best_vnd_sol)); fmt::print("VND Scheduling: {}\n", best_vnd_sol);

  sol = hill_climbing(tasks, best_sol, select2first);
  fmt::print("Total cost hill_climbing select2best: {:L}\n", evaluate(tasks, sol));

  std::ofstream schedl_out_file("gen_sol.txt");
  std::copy(std::begin(sol),
            std::end(sol),
            std::ostream_iterator<fai::Index>(schedl_out_file, "\n"));

  // sol = hill_climbing(tasks, best_sol, select2worst);
  // fmt::print("Total cost hill_climbing select2worst: {:L}\n", evaluate(tasks, sol));
}
