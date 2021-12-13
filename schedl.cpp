#include "utils.hpp"

#include <fmt/core.h>

#include <boost/range/adaptors.hpp>

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <iterator>
#include <numeric>
#include <random>
#include <unordered_set>
#include <vector>

namespace fai
{
using Sched_time = std::int64_t;
using Cost = std::int64_t;
} // namespace fai

struct Task
{
  // number of the task
  int no{};
  int exec_time{};
  int weight{1};
  int expiry_time{};

  fai::Sched_time get_delay(fai::Sched_time start_time) const
  {
    return std::max(fai::Sched_time{0}, get_sdelay(start_time));
  }

  /**
   * @brief Get the signed delay (no max with 0)
   *
   * @param start_time
   * @return fai::Sched_time
   */
  fai::Sched_time get_sdelay(fai::Sched_time start_time) const
  {
    return start_time + static_cast<fai::Sched_time>(exec_time - expiry_time);
  }

  fai::Cost get_cost(fai::Sched_time start_time) const
  {
    return static_cast<fai::Cost>(weight) * get_delay(start_time);
  }

  friend std::istream& operator>>(std::istream& in, Task& task)
  {
    in >> task.exec_time >> task.weight >> task.expiry_time;
    return in;
  }
};

std::vector<fai::Index> read_solution(std::istream& in, std::size_t nb_tasks)
{
  std::vector<fai::Index> sol;
  sol.reserve(nb_tasks);
  std::copy_n(std::istream_iterator<fai::Index>(in), nb_tasks, std::back_inserter(sol));
  return sol;
}

std::vector<Task> read_tasks(std::istream& in)
{
  int nb_task;
  in >> nb_task;
  std::vector<Task> tasks;
  tasks.reserve(nb_task);
  for (int i = 0; i < nb_task; ++i)
  {
    in >> tasks.emplace_back();
    tasks.back().no = i;
  }
  return tasks;
}

fai::Cost evaluate(std::vector<Task> const&       tasks,
                   std::vector<fai::Index> const& solution)
{
  std::unordered_set<fai::Index> uniq_sol(std::begin(solution), std::end(solution));
  if (tasks.size() != uniq_sol.size())
  {
    throw std::invalid_argument(
      fmt::format("Number of tasks {} != {} uniquely scheduled tasks",
                  tasks.size(),
                  uniq_sol.size()));
  }

  fai::Sched_time f = 0;
  fai::Sched_time curr_time = 0;

  for (fai::Index i : solution)
  {
    f += tasks[i].get_cost(curr_time);
    curr_time += tasks[i].exec_time;
  }

  return f;
}

std::vector<fai::Index> generate_random_solution(std::size_t nb_tasks)
{
  std::vector<fai::Index> sol(nb_tasks);
  std::iota(sol.begin(), sol.end(), 0);
  std::shuffle(sol.begin(), sol.end(), std::mt19937{std::random_device{}()});
  return sol;
}

template <typename Select_fn>
std::vector<fai::Index> ct_heuristic(std::vector<Task> tasks, Select_fn&& f_select)
{
  fai::Sched_time         f = 0;
  fai::Sched_time         curr_time = 0;
  std::vector<fai::Index> sol;

  while (!tasks.empty())
  {
    fai::Index task_idx = f_select(tasks, curr_time);
    Task&      task = tasks[task_idx];
    sol.push_back(task.no);
    curr_time += task.exec_time;
    if (task_idx != tasks.size() - 1)
    {
      std::swap(task, tasks.back());
    }
    tasks.pop_back();
  }

  return sol;
}

template <typename Eval_fn>
auto select(Eval_fn&& eval_fn)
{
  return
    [eval_fn = std::forward<Eval_fn>(eval_fn)](std::vector<Task> const& tasks,
                                               fai::Sched_time curr_time) -> fai::Index
  {
    fai::Index i_min = 0;
    auto       max_cost = eval_fn(tasks[i_min], curr_time);
    for (auto&& [ind, task] : tasks | adp::indexed())
    {
      if (max_cost < eval_fn(task, curr_time))
      {
        i_min = ind;
      }
    }
    return i_min;
  };
}

long double eval_sdelay_divmul_weight(Task const& task, fai::Sched_time curr_time)
{
  if (task.get_sdelay(curr_time) < 0)
  {
    return task.get_sdelay(curr_time) / static_cast<long double>(task.weight);
  }
  else
  {
    return task.get_sdelay(curr_time) * task.weight;
  }
}

long double eval_sdelay_div_weight(Task const& task, fai::Sched_time curr_time)
{
  return task.get_sdelay(curr_time) / static_cast<long double>(task.weight);
}

long double eval_static_sdelay_div_weight(Task const&                      task,
                                          [[maybe_unused]] fai::Sched_time curr_time)
{
  return task.get_sdelay(0) / static_cast<long double>(task.weight);
}

long double eval_static_sdelay_mul_weight(Task const&                      task,
                                          [[maybe_unused]] fai::Sched_time curr_time)
{
  return task.get_sdelay(0) * static_cast<long double>(task.weight);
}

long double eval_static_sdelay(Task const&                      task,
                               [[maybe_unused]] fai::Sched_time curr_time)
{
  return task.get_sdelay(0);
}

int main(int argc, char** argv)
{
  if (argc < 2)
  {
    std::cout << "usage: " << argv[0] << " tasks_file [scheduling_file]\n";
    std::exit(1);
  }
  std::ifstream           tasks_file(argv[1]);
  std::vector<Task>       tasks = read_tasks(tasks_file);
  std::vector<fai::Index> sol;
  if (argc == 3)
  {
    std::ifstream sol_file(argv[2]);
    sol = read_solution(sol_file, tasks.size());
  }
  else
  {
    sol = generate_random_solution(tasks.size());
  }
  std::cout << "Scheduling: " << sol << "\n";

  try
  {
    std::locale::global(std::locale{"en_US.UTF-8"});
  }
  catch (std::runtime_error const& e)
  {
    std::cout << "Locale en_US.UTF-8 not on your system, falling back to C"
              << "\n";
    std::locale::global(std::locale{"C"});
  }
  fmt::print("Total cost: {:L}\n", evaluate(tasks, sol));

  sol = ct_heuristic(tasks, select(eval_sdelay_divmul_weight));
  fmt::print("Total cost eval_sdelay_divmul_weight heuristic: {:L}\n",
             evaluate(tasks, sol));

  sol = ct_heuristic(tasks, select(eval_sdelay_div_weight));
  fmt::print("Total cost eval_sdelay_div_weight heuristic: {:L}\n", evaluate(tasks, sol));

  sol = ct_heuristic(tasks, select(eval_static_sdelay_div_weight));
  fmt::print("Total cost eval_static_sdelay_div_weight heuristic: {:L}\n",
             evaluate(tasks, sol));

  sol = ct_heuristic(tasks, select(eval_static_sdelay_mul_weight));
  fmt::print("Total cost eval_static_sdelay_mul_weight heuristic: {:L}\n",
             evaluate(tasks, sol));

  sol = ct_heuristic(tasks, select(eval_static_sdelay));
  fmt::print("Total cost eval_static_sdelay heuristic: {:L}\n", evaluate(tasks, sol));
}
