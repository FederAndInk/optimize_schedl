#pragma once

#include "utils.hpp"

#include <fmt/core.h>
#include <fmt/ranges.h>

#include <utility>

class Neighborhood_abstract
{
public:
  virtual ~Neighborhood_abstract() = default;

protected:
  class Polymorphic_iterator
  {
  public:
    static constexpr struct Reverse_tag
    {
    } reverse_tag;
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

    virtual fai::vector<fai::Index> const& get_current_neighbor() const noexcept = 0;

    /**
     * @brief end (could be forward or reverse)
     *
     * DO NOT Call in derived, it is replace in mixin Polymorphic_reverse_iterator
     *
     * @return true
     * @return false
     */
    [[nodiscard]] virtual bool is_end() const noexcept
    {
      return is_fend();
    }
    /**
     * @brief forward end
     *
     * @return true
     * @return false
     */
    [[nodiscard]] virtual bool is_fend() const noexcept = 0;
    [[nodiscard]] virtual bool is_rend() const noexcept = 0;
  };

  template <class Polymorphic_derived_iterator>
  class Polymorphic_reverse_iterator : public Polymorphic_derived_iterator
  {
  public:
    static_assert(std::is_base_of_v<Polymorphic_iterator, Polymorphic_derived_iterator>);

    using Polymorphic_derived_iterator::Polymorphic_derived_iterator;

    void advance() override
    {
      Polymorphic_derived_iterator::go_back();
    }

    void move_by(int dist) override
    {
      Polymorphic_derived_iterator::move_by(-dist);
    }

    void go_back() override
    {
      Polymorphic_derived_iterator::advance();
    }

    fai::vector<fai::Index> const& get_current_neighbor() const noexcept override
    {
      return Polymorphic_derived_iterator::get_current_neighbor();
    }

    [[nodiscard]] bool is_end() const noexcept override
    {
      return Polymorphic_derived_iterator::is_rend();
    }
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

  virtual Iterator begin() noexcept = 0;
  virtual Iterator rbegin() noexcept = 0;

  End_sentinel end() noexcept
  {
    return {};
  }

  End_sentinel rend() noexcept
  {
    return {};
  }

  virtual fai::vector<fai::Index>& get_base_solution() noexcept = 0;

  virtual fai::vector<fai::Index> const& get_base_solution() const noexcept = 0;

  virtual fai::Index size() const noexcept = 0;
};

class Neighborhood_base : public Neighborhood_abstract
{
private:
  fai::vector<fai::Index> base_solution;

public:
  explicit Neighborhood_base(fai::vector<fai::Index> base_solution)
    : base_solution(std::move(base_solution))
  {
  }

  fai::vector<fai::Index>& get_base_solution() noexcept override
  {
    return base_solution;
  }

  fai::vector<fai::Index> const& get_base_solution() const noexcept override
  {
    return base_solution;
  }
};

/**
 * @brief Inversion neighborhood from the first elements to the lasts
 * 1|2|3|4|5|6
 *      X->
 * 1|2|4|3|5|6
 */
class Consecutive_single_swap_neighborhood : public Neighborhood_base
{
public:
  using Neighborhood_base::Neighborhood_base;

private:
  class Iterator_derived : public Polymorphic_iterator
  {
  private:
    fai::vector<fai::Index> solution;
    fai::Index              modif_pos{0};

  public:
    explicit Iterator_derived(fai::vector<fai::Index> const& base_sol)
      : solution(base_sol)
    {
      std::swap(solution[modif_pos], solution[modif_pos + 1]);
    }
    Iterator_derived(fai::vector<fai::Index> const& base_sol, Reverse_tag)
      : solution(base_sol), modif_pos(solution.size() - 2)
    {
      std::swap(solution[modif_pos], solution[modif_pos + 1]);
    }

    void advance() override
    {
      ++modif_pos;
      if (!is_fend())
      {
        std::swap(solution[modif_pos - 1], solution[modif_pos]);
        std::swap(solution[modif_pos], solution[modif_pos + 1]);
      }
    }

    void move_by(int dist) override {}

    void go_back() override
    {
      --modif_pos;
      if (!is_rend())
      {
        std::swap(solution[modif_pos + 1], solution[modif_pos + 2]);
        std::swap(solution[modif_pos], solution[modif_pos + 1]);
      }
    }

    fai::vector<fai::Index> const& get_current_neighbor() const noexcept override
    {
      return solution;
    }

    [[nodiscard]] bool is_fend() const noexcept override
    {
      return modif_pos >= solution.size() - 1;
    }

    [[nodiscard]] bool is_rend() const noexcept override
    {
      return modif_pos < 0;
    }
  };

public:
  Iterator begin() noexcept override
  {
    return Iterator(std::make_unique<Iterator_derived>(get_base_solution()));
  }

  Iterator rbegin() noexcept override
  {
    return Iterator(std::make_unique<Polymorphic_reverse_iterator<Iterator_derived>>(
      get_base_solution(),
      Iterator_derived::reverse_tag));
  }

  fai::Index size() const noexcept override
  {
    return (get_base_solution().size() - 1);
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
      if (!is_fend())
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

    fai::vector<fai::Index> const& get_current_neighbor() const noexcept override
    {
      return solution;
    }

    [[nodiscard]] bool is_fend() const noexcept override
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

  Iterator rbegin() noexcept override
  {
    return Iterator(std::make_unique<Polymorphic_reverse_iterator<Iterator_derived>>(
      get_base_solution(),
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
class Backward_neighborhood : public Neighborhood_abstract
{
private:
  Neighborhood nbh;

public:
  using Base_neighborhood = Neighborhood;

  explicit Backward_neighborhood(fai::vector<fai::Index> base_solution)
    : nbh(std::move(base_solution))
  {
  }

  Iterator begin() noexcept override
  {
    return std::rbegin(nbh);
  }

  Iterator rbegin() noexcept override
  {
    return std::begin(nbh);
  }

  fai::vector<fai::Index>& get_base_solution() noexcept override
  {
    return nbh.get_base_solution();
  }

  fai::vector<fai::Index> const& get_base_solution() const noexcept override
  {
    return nbh.get_base_solution();
  }

  fai::Index size() const noexcept override
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
