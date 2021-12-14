#pragma once

#include "Task.hpp"

#include <vector>

class Consecutive_single_swap_neighborhood
{
private:
  std::vector<fai::Index> base_solution;

public:
  Consecutive_single_swap_neighborhood(std::vector<fai::Index> base_solution)
    : base_solution(std::move(base_solution))
  {
  }

  class Iterator
  {
  private:
    std::vector<fai::Index> solution;
    fai::Index              modif_pos{0};

  public:
    Iterator(fai::Index pos) : modif_pos(pos) {}

    Iterator(std::vector<fai::Index> const& base_sol) : solution(base_sol)
    {
      std::swap(solution[modif_pos], solution[modif_pos + 1]);
    }

    Iterator& operator++()
    {
      modif_pos++;
      if (modif_pos < solution.size() - 1)
      {
        std::swap(solution[modif_pos - 1], solution[modif_pos]);
        std::swap(solution[modif_pos], solution[modif_pos + 1]);
      }
      return *this;
    }

    std::vector<fai::Index> const& operator*()
    {
      return solution;
    }

    bool operator==(Iterator const& rhs) const
    {
      return modif_pos == rhs.modif_pos;
    }

    bool operator!=(Iterator const& rhs) const
    {
      return !(*this == rhs);
    }
  };

  Iterator begin()
  {
    return Iterator(base_solution);
  }

  Iterator end()
  {
    return Iterator(base_solution.size() - 1);
  }
};
