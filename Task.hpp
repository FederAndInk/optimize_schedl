#pragma once

#include "fmt/core.h"
#include "utils.hpp"

#include <algorithm>
#include <cstdint>
#include <istream>
#include <iterator>
#include <stdexcept>
#include <unordered_set>

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

  [[nodiscard]] fai::Sched_time get_delay(fai::Sched_time start_time) const
  {
    return std::max(fai::Sched_time{0}, get_sdelay(start_time));
  }

  /**
   * @brief Get the signed delay (no max with 0)
   *
   * @param start_time
   * @return fai::Sched_time
   */
  [[nodiscard]] fai::Sched_time get_sdelay(fai::Sched_time start_time) const
  {
    return start_time + static_cast<fai::Sched_time>(exec_time - expiry_time);
  }

  [[nodiscard]] fai::Cost get_cost(fai::Sched_time start_time) const
  {
    return static_cast<fai::Cost>(weight) * get_delay(start_time);
  }

  friend std::istream& operator>>(std::istream& in, Task& task)
  {
    in >> task.exec_time >> task.weight >> task.expiry_time;
    return in;
  }
};

inline fai::Cost evaluate(fai::vector<Task> const&       tasks,
                          fai::vector<fai::Index> const& solution)
{
  std::unordered_set<fai::Index> uniq_sol(std::begin(solution), std::end(solution));
  if (tasks.size() != fai::ssize(uniq_sol))
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