#pragma once

#include "utils.hpp"

#include <utility>

/**
 * @brief Inversion neighborhood from the first elements to the lasts
 * 1|2|3|4|5|6
 *      X->
 * 1|2|4|3|5|6
 */
class Consecutive_single_swap_neighborhood
{
private:
  fai::vector<fai::Index> base_solution;

public:
  explicit Consecutive_single_swap_neighborhood(fai::vector<fai::Index> base_solution)
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

    explicit Iterator(fai::vector<fai::Index> const& base_sol) : solution(base_sol)
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

    fai::vector<fai::Index> const& operator*() const noexcept
    {
      return solution;
    }

    bool operator==(Iterator const& rhs) const noexcept
    {
      return modif_pos == rhs.modif_pos;
    }

    bool operator!=(Iterator const& rhs) const noexcept
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

  fai::vector<fai::Index>& get_base_solution() noexcept
  {
    return base_solution;
  }

  fai::Index size() const noexcept
  {
    return (base_solution.size() - 1);
  }
};

/**
 * @brief Inversion neighborhood from the last elements to the firsts
 * 1|2|3|4|5|6
 *    <-X
 * 1|2|4|3|5|6
 */
class Reverse_consecutive_single_swap_neighborhood
{
private:
  fai::vector<fai::Index> base_solution;

public:
  explicit Reverse_consecutive_single_swap_neighborhood(
    fai::vector<fai::Index> base_solution)
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

    explicit Iterator(fai::vector<fai::Index> const& base_sol)
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

    fai::vector<fai::Index> const& operator*() const noexcept
    {
      return solution;
    }

    bool operator==(Iterator const& rhs) const noexcept
    {
      return modif_pos == rhs.modif_pos;
    }

    bool operator!=(Iterator const& rhs) const noexcept
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

  fai::vector<fai::Index>& get_base_solution() noexcept
  {
    return base_solution;
  }

  fai::Index size() const noexcept
  {
    return (base_solution.size() - 1);
  }
};

/**
 * @brief reverse subrange
 *
 * beginning at the first element, then growing the range from 2 to n elements
 * then redo it for the second, the third...
 *
 * 1|2|3|4|5|6
 *    \|/
 *     X
 *    /|\
 * 1|4|3|2|5|6
 *
 */
class Reverse_neighborhood
{
private:
  fai::vector<fai::Index> base_solution;

public:
  explicit Reverse_neighborhood(fai::vector<fai::Index> base_solution)
    : base_solution(std::move(base_solution))
  {
  }

  class Iterator
  {
  private:
    fai::vector<fai::Index> solution;
    fai::Index              modif_pos_beg{0};
    fai::Index              modif_pos_end{modif_pos_beg + 2};

  public:
    explicit Iterator(fai::Index beg_pos) : modif_pos_beg(beg_pos) {}

    Iterator(fai::vector<fai::Index> const& base_sol) : solution(base_sol)
    {
      std::reverse(std::next(solution.begin(), modif_pos_beg),
                   std::next(solution.begin(), modif_pos_end));
    }

    Iterator& operator++()
    {
      next_range();
      if (valid_range())
      {
        reverse_sel_range();
      }
      return *this;
    }

    fai::vector<fai::Index> const& operator*() const noexcept
    {
      return solution;
    }

    bool operator==(Iterator const& rhs) const noexcept
    {
      return modif_pos_beg == rhs.modif_pos_beg && modif_pos_end == rhs.modif_pos_end;
    }

    bool operator!=(Iterator const& rhs) const noexcept
    {
      return !(*this == rhs);
    }

  private:
    [[nodiscard]] bool valid_range() const noexcept
    {
      return modif_pos_beg < solution.size() - 1;
    }

    void next_range() noexcept
    {
      ++modif_pos_end;
      if (modif_pos_end == solution.size() + 1)
      {
        // restore
        std::reverse(std::next(std::begin(solution), modif_pos_beg), std::end(solution));
        ++modif_pos_beg;
        modif_pos_end = modif_pos_beg + 2;
      }
    }

    void reverse_sel_range()
    {
      // rotate right
      auto rbegin =
        std::make_reverse_iterator(std::next(std::begin(solution), modif_pos_end));
      std::rotate(rbegin, rbegin + 1, std::prev(std::rend(solution), modif_pos_beg));
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

  fai::Index size() const noexcept
  {
    fai::Index n = base_solution.size();
    return n * (n - 1) / 2;
  }

  fai::vector<fai::Index>& get_base_solution() noexcept
  {
    return base_solution;
  }
};
