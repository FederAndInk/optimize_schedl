#pragma once

#include "Task.hpp"
#include "utils.hpp"

#include <boost/range/adaptors.hpp>

#include <vector>

/**
 * return a function to find the best of a vector of
 * Taskfrom a function witch compare two Tasks.
 */
template <typename Eval_fn>
auto select(Eval_fn&& eval_fn)
{
  return
    [eval_fn = std::forward<Eval_fn>(eval_fn)](std::vector<Task> const& tasks,
                                               fai::Sched_time curr_time) -> fai::Index
  {
    fai::Index i_max = 0;
    auto       max_cost = eval_fn(tasks[i_max], curr_time);
    for (auto&& [ind, task] : tasks | adp::indexed())
    {
      if (max_cost < eval_fn(task, curr_time))
      {
        i_max = ind;
      }
    }
    return i_max;
  };
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

/*
 * if late:
 *   lateness*weight
 * else:
 *   lead/weight
 */
inline long double eval_sdelay_divmul_weight(Task const& task, fai::Sched_time curr_time)
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

inline long double eval_sdelay_div_weight(Task const& task, fai::Sched_time curr_time)
{
  return task.get_sdelay(curr_time) / static_cast<long double>(task.weight);
}

/*
 * (lateness_lead)/weight
 */
inline long double
eval_static_sdelay_div_weight(Task const&                      task,
                              [[maybe_unused]] fai::Sched_time curr_time)
{
  return task.get_sdelay(0) / static_cast<long double>(task.weight);
}

inline long double
eval_static_sdelay_mul_weight(Task const&                      task,
                              [[maybe_unused]] fai::Sched_time curr_time)
{
  return task.get_sdelay(0) * static_cast<long double>(task.weight);
}

inline long double eval_static_sdelay(Task const&                      task,
                                      [[maybe_unused]] fai::Sched_time curr_time)
{
  return task.get_sdelay(0);
}

inline long double eval_static_expiry(Task const&                      task,
                                      [[maybe_unused]] fai::Sched_time curr_time)
{
  return -task.expiry_time;
}

inline long double
eval_static_expiry_div_weight_mul_time(Task const&                      task,
                                       [[maybe_unused]] fai::Sched_time curr_time)
{
  return -task.expiry_time * task.exec_time / static_cast<long double>(task.weight);
}

struct Function_reflect
{
  using Heuristic_fn = long double (*)(Task const& task, fai::Sched_time curr_time);
  std::string_view name;
  Heuristic_fn     fn;
};

#define reflect_fn(fn) #fn, fn

inline std::vector<Function_reflect> get_heuristics()
{
  static std::vector<Function_reflect> heuristics{
    {reflect_fn(eval_sdelay_divmul_weight)},
    {reflect_fn(eval_sdelay_div_weight)},
    {reflect_fn(eval_static_sdelay_div_weight)},
    {reflect_fn(eval_static_sdelay_mul_weight)},
    {reflect_fn(eval_static_sdelay)},
    {reflect_fn(eval_static_expiry)},
    {reflect_fn(eval_static_expiry_div_weight_mul_time)} //
  };
  return heuristics;
}

#undef reflect_fn