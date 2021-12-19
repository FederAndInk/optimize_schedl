#include "Task.hpp"
#include "heuristics.hpp"
#include "local_search.hpp"
#include "utils.hpp"

#include <fmt/core.h>
#include <fmt/ranges.h>

#include <algorithm>
#include <csignal>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <iterator>
#include <locale>
#include <numeric>
#include <random>
#include <stdexcept>

namespace fs = std::filesystem;

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

void treat_solution(fai::vector<Task> const&  tasks,
                    fai::vector<fai::Index>&& sol,
                    std::string const&        name,
                    std::string_view          desc)
{
  fai::Cost cost = evaluate(tasks, sol);
  fmt::print("Total cost {}: {:L}\n", desc, cost);

  std::filesystem::create_directory("sols");
  std::ofstream schedl_out_file(fmt::format("sols/gen_sol_{}_{}.txt", cost, name));
  if (!schedl_out_file)
  {
    fmt::print("error with file: {}\n", "sols/gen_sol_" + name + ".txt");
  }
  else
  {
    std::copy(std::begin(sol),
              std::end(sol),
              std::ostream_iterator<fai::Index>(schedl_out_file, "\n"));
  }
}

extern "C" void interrupt_handler(int)
{
  fai::stop_request() = true;
}

int main(int argc, char** argv)
{
  using namespace std::literals;
  if (argc < 2)
  {
    fmt::print("usage: {} tasks_file [scheduling_file]\n", argv[0]);
    return 1;
  }

  std::signal(SIGINT, interrupt_handler);

  std::ifstream tasks_file(argv[1]);
  if (!tasks_file)
  {
    fmt::print("Error opening file {}", argv[1]);
    return 1;
  }
  fai::vector<Task> const tasks = read_tasks(tasks_file);
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

  std::iota(std::begin(sol), std::end(sol), 0);
  if (fai::Cost given_ord_cost = evaluate(tasks, sol); //
      best_sol_cost > given_ord_cost)
  {
    best_sol = sol;
    best_sol_cost = given_ord_cost;
    best_algo = "given order";
  }

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

  auto rcssn_sol = std::async(
    std::launch::async,
    [&tasks, &best_sol]()
    {
      return hill_climbing<Reverse_consecutive_single_swap_neighborhood>(tasks,
                                                                         best_sol,
                                                                         select2best);
    });
  auto cssn_sol =
    std::async(std::launch::async,
               [&tasks, &best_sol]()
               {
                 return hill_climbing<Consecutive_single_swap_neighborhood>(tasks,
                                                                            best_sol,
                                                                            select2best);
               });
  auto brn_sol = std::async(
    std::launch::async,
    [&tasks, &best_sol]()
    {
      return hill_climbing<Backward_neighborhood<Reverse_neighborhood>>(tasks,
                                                                        best_sol,
                                                                        select2best);
    });
  auto rn_sol =
    std::async(std::launch::async,
               [&tasks, &best_sol]() {
                 return hill_climbing<Reverse_neighborhood>(tasks, best_sol, select2best);
               });

  auto base_out_fname = fs::path(argv[1]).stem().string();

  treat_solution(tasks,
                 rcssn_sol.get(),
                 base_out_fname + "_hc_best_rcssn"s,
                 "Hill climbing select2best"
                 " Reverse_consecutive_single_swap_neighborhood");
  treat_solution(tasks,
                 cssn_sol.get(),
                 base_out_fname + "_hc_best_cssn"s,
                 "Hill climbing select2best Consecutive_single_swap_neighborhood");
  treat_solution(
    tasks,
    brn_sol.get(),
    base_out_fname + "_hc_best_brn"s,
    fmt::format("Hill climbing select2best {}",
                get_neighborhood_name<Backward_neighborhood<Reverse_neighborhood>>()));
  treat_solution(tasks,
                 rn_sol.get(),
                 base_out_fname + "_hc_best_rn"s,
                 fmt::format("Hill climbing select2best {}",
                             get_neighborhood_name<Reverse_neighborhood>()));

  // sol = hill_climbing(tasks, best_sol, select2worst);
  // fmt::print("Total cost hill_climbing select2worst: {:L}\n", evaluate(tasks, sol));
}
