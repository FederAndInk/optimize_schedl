#include <fmt/format.h>

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <iterator>
#include <unordered_set>
#include <vector>

namespace fai
{
using Index = int;
using Sched_time = std::int64_t;
} // namespace fai

struct Task
{
  int exec_time{};
  int weight{};
  int expiry_time{};

  fai::Sched_time get_delay(fai::Sched_time start_time) const
  {
    return std::max(fai::Sched_time{0},
                    start_time + static_cast<fai::Sched_time>(exec_time - expiry_time));
  }

  fai::Sched_time get_cost(fai::Sched_time start_time) const
  {
    return static_cast<fai::Sched_time>(weight) * get_delay(start_time);
  }

  friend std::istream& operator>>(std::istream& in, Task& task)
  {
    in >> task.exec_time >> task.weight >> task.expiry_time;
    return in;
  }
};

std::vector<Task> read_tasks(std::istream& in)
{
  std::size_t nb_task;
  in >> nb_task;
  std::vector<Task> tasks;
  tasks.reserve(nb_task);
  for (std::size_t i = 0; i < nb_task; ++i)
  {
    in >> tasks.emplace_back();
  }
  return tasks;
}

fai::Sched_time evaluate(std::vector<Task> const&       tasks,
                         std::vector<fai::Index> const& solution)
{
  std::unordered_set<fai::Index> uniq_sol(std::begin(solution), std::end(solution));
  if (tasks.size() == uniq_sol.size())
  {
    throw std::invalid_argument(
      fmt::format("Number of tasks {} != {} uniquely scheduled tasks",
                  tasks.size(),
                  solution.size()));
  }

  fai::Sched_time f = 0;
  fai::Sched_time curr_time = 0;

  for (fai::Index i : solution)
  {
    f += tasks[i].get_cost(curr_time);
    curr_time += tasks[i].exec_time;
    std::cout << "+" << i << " t: " << curr_time << " f: " << f << "\n";
  }

  return f;
}

int main(int argc, char** argv)
{
  if (argc != 2)
  {
    std::cout << "usage: " << argv[0] << " tasks_file\n";
    std::exit(1);
  }
  std::ifstream           tasks_file(argv[1]);
  std::vector<Task>       tasks = read_tasks(tasks_file);
  std::vector<fai::Index> sol;
  std::copy_n(std::istream_iterator<fai::Index>(std::cin),
              tasks.size(),
              std::back_inserter(sol));
  std::cout << "Total cost: " << evaluate(tasks, sol) << "\n";
}
