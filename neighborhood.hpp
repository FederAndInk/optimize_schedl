#pragma once

#include "utils.hpp"

#include <fmt/core.h>
#include <fmt/ranges.h>

#include <utility>

class Neighborhood_base
{
private:
  fai::vector<fai::Index> base_solution;

public:
  explicit Neighborhood_base(fai::vector<fai::Index> base_solution)
    : base_solution(std::move(base_solution))
  {
  }

  virtual ~Neighborhood_base() = default;

protected:
  class Polymorphic_iterator
  {
  public:
    virtual ~Polymorphic_iterator() = default;

    virtual void advance()
    {
      move_by(1);
    }

    virtual void move_by(int dist) = 0;

    virtual void go_back()
    {
      move_by(-1);
    }

    virtual fai::vector<fai::Index> const& get_current_neighbor() = 0;

    [[nodiscard]] virtual bool is_end() const noexcept = 0;
    [[nodiscard]] virtual bool is_rend() const noexcept = 0;
  };

public:
  struct End_sentinel
  {
  };

  class Iterator
  {
  private:
    std::unique_ptr<Polymorphic_iterator> it;

  public:
    Iterator(std::unique_ptr<Polymorphic_iterator> it) : it(std::move(it)) {}

    Iterator& operator++()
    {
      it->advance();
      return *this;
    }

    Iterator& operator+=(int dist)
    {
      it->move_by(dist);
      return *this;
    }

    // fai::vector<fai::Index> const& operator[](int dist) const
    // {
    //   it->move_by(dist);
    //   return it->get_current_neighbor();
    // }

    fai::vector<fai::Index> const& operator*() const noexcept
    {
      return it->get_current_neighbor();
    }

    bool operator==(End_sentinel rhs) const noexcept
    {
      return it->is_end();
    }

    bool operator!=(End_sentinel rhs) const noexcept
    {
      return !(*this == rhs);
    }
  };

  class Reverse_iterator
  {
  private:
    std::unique_ptr<Polymorphic_iterator> it;

  public:
    Reverse_iterator(std::unique_ptr<Polymorphic_iterator> it) : it(std::move(it)) {}

    Reverse_iterator& operator++()
    {
      it->go_back();
      return *this;
    }

    fai::vector<fai::Index> const& operator*() const noexcept
    {
      return it->get_current_neighbor();
    }

    bool operator==(End_sentinel rhs) const noexcept
    {
      return it->is_rend();
    }

    bool operator!=(End_sentinel rhs) const noexcept
    {
      return !(*this == rhs);
    }
  };

  virtual Iterator         begin() noexcept = 0;
  virtual Reverse_iterator rbegin() noexcept = 0;

  End_sentinel end() noexcept
  {
    return {};
  }

  End_sentinel rend() noexcept
  {
    return {};
  }

  fai::vector<fai::Index>& get_base_solution() noexcept
  {
    return base_solution;
  }

  fai::vector<fai::Index> const& get_base_solution() const noexcept
  {
    return base_solution;
  }

  virtual fai::Index size() const noexcept = 0;
};

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

  Iterator begin() noexcept
  {
    return Iterator(base_solution);
  }

  Iterator end() noexcept
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

  Iterator begin() noexcept
  {
    return Iterator(base_solution);
  }

  Iterator end() noexcept
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
class Reverse_neighborhood : public Neighborhood_base
{
public:
  using Neighborhood_base::Neighborhood_base;

private:
  class Iterator_derived : public Polymorphic_iterator
  {
  private:
    fai::vector<fai::Index> solution;
    fai::Index              modif_pos_beg{0};
    fai::Index              modif_pos_end{modif_pos_beg + 2};

  public:
    static constexpr struct Reverse_tag
    {
    } reverse_tag;

    Iterator_derived(fai::vector<fai::Index> const& base_sol) : solution(base_sol)
    {
      std::reverse(std::next(solution.begin(), modif_pos_beg),
                   std::next(solution.begin(), modif_pos_end));
    }

    Iterator_derived(fai::vector<fai::Index> const& base_sol, Reverse_tag)
      : solution(base_sol), modif_pos_beg(base_sol.size() - 2)
    {
      std::reverse(std::next(solution.begin(), modif_pos_beg),
                   std::next(solution.begin(), modif_pos_end));
    }

    void advance() override
    {
      next_range();
      if (!is_end())
      {
        inc_reversed_range();
      }
    }

    void move_by(int dist) override
    {
      // restore
      reverse_range();
      for (int i = 0; i < dist; ++i)
      {
        ++modif_pos_end;
        if (modif_pos_end == solution.size() + 1)
        {
          ++modif_pos_beg;
          modif_pos_end = modif_pos_beg + 2;
        }
      }
      reverse_range();
    }

    void go_back() override
    {
      --modif_pos_end;
      if (modif_pos_end == modif_pos_beg + 1)
      {
        // restore
        std::swap(solution[modif_pos_beg], solution[modif_pos_beg + 1]);
        --modif_pos_beg;
        modif_pos_end = solution.size();
        if (!is_rend())
        {
          // reverse all in subrange
          std::reverse(std::next(std::begin(solution), modif_pos_beg),
                       std::end(solution));
        }
      }
      else
      {
        // rotate left
        auto beg = std::next(std::begin(solution), modif_pos_beg);
        std::rotate(beg, beg + 1, std::next(std::begin(solution), modif_pos_end + 1));
      }
    }

    fai::vector<fai::Index> const& get_current_neighbor() override
    {
      return solution;
    }

    [[nodiscard]] bool is_end() const noexcept override
    {
      return modif_pos_beg >= solution.size() - 1;
    }

    [[nodiscard]] bool is_rend() const noexcept override
    {
      return modif_pos_beg < 0;
    }

  private:
    void next_range() noexcept
    {
      ++modif_pos_end;
      if (modif_pos_end == solution.size() + 1)
      {
        --modif_pos_end;
        reverse_range();

        ++modif_pos_beg;
        modif_pos_end = modif_pos_beg + 2;
      }
    }

    void inc_reversed_range()
    {
      // rotate right
      auto rbegin =
        std::make_reverse_iterator(std::next(std::begin(solution), modif_pos_end));
      std::rotate(rbegin, rbegin + 1, std::prev(std::rend(solution), modif_pos_beg));
    }

    void reverse_range()
    {
      std::reverse(std::next(std::begin(solution), modif_pos_beg),
                   std::next(std::begin(solution), modif_pos_end));
    }
  };

public:
  Iterator begin() noexcept override
  {
    return Iterator(std::make_unique<Iterator_derived>(get_base_solution()));
  }

  Reverse_iterator rbegin() noexcept override
  {
    return Reverse_iterator(
      std::make_unique<Iterator_derived>(get_base_solution(),
                                         Iterator_derived::reverse_tag));
  }

  // Iterator end()
  // {
  //   return Iterator(base_solution.size() - 1);
  // }

  fai::Index size() const noexcept override
  {
    fai::Index n = get_base_solution().size();
    return n * (n - 1) / 2;
  }
};

template <typename Neighborhood>
class Backward_neighborhood
{
private:
  Neighborhood nbh;

public:
  using Base_neighborhood = Neighborhood;

  explicit Backward_neighborhood(fai::vector<fai::Index> base_solution)
    : nbh(std::move(base_solution))
  {
  }

  auto begin() noexcept
  {
    return std::rbegin(nbh);
  }

  auto end() noexcept
  {
    return std::rend(nbh);
  }

  fai::vector<fai::Index>& get_base_solution() noexcept
  {
    return nbh.get_base_solution();
  }

  fai::vector<fai::Index> const& get_base_solution() const noexcept
  {
    return nbh.get_base_solution();
  }

  fai::Index size() const noexcept
  {
    return nbh.size();
  }
};

template <typename U>
struct Is_backward_neighborhood
{
private:
  template <typename T>
  static constexpr std::false_type test(T);

  template <typename T>
  static constexpr std::true_type test(Backward_neighborhood<T>);

public:
  static constexpr bool value = decltype(test(std::declval<U>()))::value;
};

template <typename U>
constexpr auto is_backward_neighborhood_v = Is_backward_neighborhood<U>::value;

template <typename U>
struct base_neighborhood
{
private:
  template <typename T>
  static constexpr T test(T);

  template <typename T>
  static constexpr T test(Backward_neighborhood<T>);

public:
  using type = decltype(test(std::declval<U>()));
};

template <typename U>
using base_neighborhood_t = typename base_neighborhood<U>::type;

template <typename Neighborhood>
std::string get_neighborhood_name()
{
  std::string ret;
  if constexpr (is_backward_neighborhood_v<Neighborhood>)
  {
    ret = "Backward ";
  }
  using Base_neighborhood = base_neighborhood_t<Neighborhood>;
  if constexpr (std::is_same_v<Base_neighborhood, Consecutive_single_swap_neighborhood>)
  {
    ret += "Consecutive_single_swap_neighborhood";
  }
  else if (std::is_same_v<Base_neighborhood,
                          Reverse_consecutive_single_swap_neighborhood>)
  {
    ret += "Reverse_consecutive_single_swap_neighborhood";
  }
  else if (std::is_same_v<Base_neighborhood, Reverse_neighborhood>)
  {
    ret += "Reverse_neighborhood";
  }
  else
  {
    ret += "unknown";
  }

  return ret;
}
