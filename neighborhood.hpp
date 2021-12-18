#pragma once

#include "utils.hpp"

#include <utility>

class Consecutive_single_swap_neighborhood
{
private:
  fai::vector<fai::Index> base_solution;

public:
  Consecutive_single_swap_neighborhood(fai::vector<fai::Index> base_solution)
    : base_solution(std::move(base_solution))
  {
  }

  class Iterator
  {
  private:
    fai::vector<fai::Index> solution;
    fai::Index              modif_pos{0};

  public:
    Iterator(fai::Index pos) : modif_pos(pos) {}

    Iterator(fai::vector<fai::Index> const& base_sol) : solution(base_sol)
    {
      std::swap(solution[modif_pos], solution[modif_pos + 1]);
    }

    Iterator& operator++()
    {
      ++modif_pos;
      if (modif_pos < solution.size() - 1)
      {
        std::swap(solution[modif_pos - 1], solution[modif_pos]);
        std::swap(solution[modif_pos], solution[modif_pos + 1]);
      }
      return *this;
    }

    fai::vector<fai::Index> const& operator*()
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

  fai::vector<fai::Index>& get_base_solution()
  {
    return base_solution;
  }
};

class Reverse_consecutive_single_swap_neighborhood
{
private:
  fai::vector<fai::Index> base_solution;

public:
  Reverse_consecutive_single_swap_neighborhood(fai::vector<fai::Index> base_solution)
    : base_solution(std::move(base_solution))
  {
  }

  class Iterator
  {
  private:
    fai::vector<fai::Index> solution;
    fai::Index              modif_pos{0};

  public:
    Iterator() = default;

    Iterator(fai::vector<fai::Index> const& base_sol)
      : solution(base_sol), modif_pos(solution.size() - 1)
    {
      std::swap(solution[modif_pos], solution[modif_pos - 1]);
    }

    Iterator& operator++()
    {
      --modif_pos;
      if (modif_pos > 0)
      {
        std::swap(solution[modif_pos + 1], solution[modif_pos]);
        std::swap(solution[modif_pos], solution[modif_pos - 1]);
      }
      return *this;
    }

    fai::vector<fai::Index> const& operator*()
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
    return {};
  }

  fai::vector<fai::Index>& get_base_solution()
  {
    return base_solution;
  }
};
