#include "Task.hpp"
#include "heuristics.hpp"
#include "iterated_local_search.hpp"
#include "local_search.hpp"
#include "utils.hpp"

#include <fmt/core.h>
#include <fmt/ranges.h>

#include <boost/program_options.hpp>

#include <algorithm>
#include <csignal>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <iterator>
#include <list>
#include <locale>
#include <numeric>
#include <random>
#include <stdexcept>

namespace fs = std::filesystem;
namespace po = boost::program_options;
using namespace std::literals;

Scheduling read_solution(std::istream& in, fai::Index nb_tasks)
{
  Scheduling sol;
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

Scheduling generate_random_solution(fai::Index nb_tasks)
{
  Scheduling sol(nb_tasks);
  std::iota(sol.begin(), sol.end(), 0);
  std::shuffle(sol.begin(), sol.end(), std::mt19937{std::random_device{}()});
  return sol;
}

void treat_solution(fai::vector<Task> const& tasks,
                    Scheduling&&             sol,
                    std::string const&       base_name,
                    std::string const&       short_details,
                    std::string_view         desc)
{
  fai::Cost cost = evaluate(tasks, sol);
  fmt::print("Total cost {}: {:L}\n", desc, cost);

  std::filesystem::create_directory("sols");
  auto fname = fmt::format("sols/gen_sol_{}_{}_{}.txt", base_name, cost, short_details);
  std::ofstream schedl_out_file(fname);
  if (!schedl_out_file)
  {
    fmt::print("error with file: {}\n", fname);
  }
  else
  {
    std::copy(std::begin(sol),
              std::end(sol),
              std::ostream_iterator<fai::Index>(schedl_out_file, "\n"));
  }
}

template <typename Neighborhood, typename Select_fn>
auto launch(fai::vector<Task> const& tasks,
            Scheduling const&        sol,
            std::string const&       base_name,
            Select_fn&&              select_fn)
{
  return std::async(std::launch::async,
                    [&]()
                    {
                      auto gen_sol = hill_climbing<Neighborhood>(tasks, sol, select_fn);
                      treat_solution(
                        tasks,
                        std::move(gen_sol),
                        base_name,
                        fmt::format("_hc_{}_{}",
                                    select_fn_name(select_fn),
                                    get_neighborhood_short_name<Neighborhood>()),
                        fmt::format("Hill climbing {} {}",
                                    select_fn_name(select_fn),
                                    get_neighborhood_name<Neighborhood>()));
                    });
}

std::atomic<int> nb_ctrl_c = 0;
extern "C" void  interrupt_handler(int)
{
  fai::stop_request() = true;
  ++nb_ctrl_c;
  if (nb_ctrl_c > 3)
  {
    std::exit(130);
  }
}

int main(int argc, char** argv)
{
  if (argc < 2)
  {
    fmt::print("usage: {} tasks_file [scheduling_file]\n", argv[0]);
    fmt::print("{0} <problem_file> --sol <solution_file>\n"
               "{0} <problem_file> --heuristics\n"
               "{0} <problem_file> --random\n"
               "{0} <problem_file> --hc [--sol <solution_file>|--random]\n"
               "{0} <problem_file> --ils [--sol <solution_file>|--random]\n",
               argv[0]);
    return 1;
  }

  std::signal(SIGINT, interrupt_handler);

  std::string problem_file_name;
  std::string sol_file_name;

  po::options_description desc("Options");
  desc.add_options()("help", "produce help message")                               //
    ("sol", po::value<std::string>(&sol_file_name), "provide scheduling solution") //
    ("heuristics", "compute heuristics")                                           //
    ("random", "generate random scheduling")                                       //
    ("hc", "hill climbing")                                                        //
    ("ils", "Iterated local search")                                               //
    ("problem_file",
     po::value<std::string>(&problem_file_name)->required(),
     "Problem file") //
    ;

  po::positional_options_description pos_args;
  pos_args.add("problem_file", 1);
  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(desc).positional(pos_args).run(),
            vm);
  po::notify(vm);

  try
  {
    std::locale::global(std::locale{"en_US.UTF-8"});
  }
  catch (std::runtime_error const& e)
  {
    fmt::print(
      "Warning: Locale en_US.UTF-8 not available on your system, falling back to C\n");
    std::locale::global(std::locale{"C"});
  }

  std::ifstream tasks_file(problem_file_name);
  if (!tasks_file)
  {
    fmt::print("Error opening file {}", problem_file_name);
    return 1;
  }
  fai::vector<Task> const tasks = read_tasks(tasks_file);

  std::string_view best_algo = "undefined";
  Scheduling       best_sol;
  fai::Cost        best_sol_cost;

  if (vm.count("sol"))
  {
    std::ifstream sol_file(sol_file_name);
    if (!sol_file)
    {
      fmt::print("Error opening file {}", sol_file_name);
      return 1;
    }
    best_sol = read_solution(sol_file, tasks.size());
    best_algo = "user provided";
    best_sol_cost = evaluate(tasks, best_sol);
    fmt::print("User provided Scheduling: {}\n", best_sol);
    fmt::print("{} Total cost: {:L}\n", best_algo, best_sol_cost);
  }
  auto rand_sol = generate_random_solution(tasks.size());
  auto rand_cost = evaluate(tasks, rand_sol);
  if (!vm.count("sol") || rand_cost < best_sol_cost)
  {
    best_sol_cost = rand_cost;
    best_sol = rand_sol;
    best_algo = "random";
  }
  if (vm.count("random"))
  {
    fmt::print("Random Scheduling: {}\n", rand_sol);
    fmt::print("Random Total cost: {:L}\n", rand_cost);
  }

  Scheduling ord_sol(tasks.size());
  std::iota(std::begin(ord_sol), std::end(ord_sol), 0);
  if (fai::Cost given_ord_cost = evaluate(tasks, ord_sol); //
      best_sol_cost > given_ord_cost)
  {
    best_sol = ord_sol;
    best_sol_cost = given_ord_cost;
    best_algo = "given order";
    fmt::print("Given order Total cost: {:L}\n", given_ord_cost);
  }

  for (auto&& heuristic : get_heuristics())
  {
    auto sol = ct_heuristic(tasks, select(heuristic.fn));
    auto sol_cost = evaluate(tasks, sol);
    if (vm.count("heuristics"))
    {
      fmt::print("Total cost {} heuristic: {:L}\n", heuristic.name, sol_cost);
    }

    if (best_sol_cost > sol_cost)
    {
      best_sol = sol;
      best_algo = heuristic.name;
      best_sol_cost = sol_cost;
    }
  }
  fmt::print("\nBest algo: {} with cost: {:L}\n", best_algo, best_sol_cost);

  auto base_out_fname = fs::path(problem_file_name).stem().string();
  if (vm.count("ils"))
  {
    using Local_search_nbh = Sliding_reverse_neighborhood<15>;
    using Perturbation_nbh = Sliding_reverse_neighborhood<20>;
    auto sol_ils = ils(
      tasks,
      best_sol,
      [](fai::vector<Task> const& tasks, Scheduling&& base_solution) {
        return hill_climbing<Local_search_nbh>(tasks,
                                               std::move(base_solution),
                                               select2best);
      },
      [](Scheduling& solution, std::vector<Scheduling>& history)
      { return random_distant_neighbor<Perturbation_nbh>(solution, 30, history); },
      accept_best,
      stop_n_worse<20>);

    treat_solution(
      tasks,
      std::move(sol_ils),
      base_out_fname,
      fmt::format("_ils_best_hc_best_{}_pert_",
                  get_neighborhood_short_name<Local_search_nbh>(),
                  get_neighborhood_short_name<Perturbation_nbh>()),
      fmt::format(
        "ILS (HC select2best {}) accept_best stop_n_worse<20> perturb: rand<30>neigh {}",
        get_neighborhood_name<Local_search_nbh>(),
        get_neighborhood_name<Perturbation_nbh>()));
  }

  if (fai::stop_request())
  {
    return 130;
  }

  if (vm.count("hc"))
  {
    std::vector<std::future<void>> compute_tasks;
    compute_tasks.push_back(
      launch<Backward_neighborhood<Consecutive_single_swap_neighborhood>>(tasks,
                                                                          best_sol,
                                                                          base_out_fname,
                                                                          select2best));
    compute_tasks.push_back(launch<Consecutive_single_swap_neighborhood>(tasks,
                                                                         best_sol,
                                                                         base_out_fname,
                                                                         select2first));
    compute_tasks.push_back(
      launch<Backward_neighborhood<Reverse_neighborhood>>(tasks,
                                                          best_sol,
                                                          base_out_fname,
                                                          select2best));
    compute_tasks.push_back(
      launch<Reverse_neighborhood>(tasks, best_sol, base_out_fname, select2first));
  }
  // sol = hill_climbing(tasks, best_sol, select2worst);
  // fmt::print("Total cost hill_climbing select2worst: {:L}\n", evaluate(tasks, sol));
}
