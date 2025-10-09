// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common.hpp"
#include "counting_iterator.hpp"
#include "iterator_facade.hpp"
#include "typing.hpp"

#include <algorithm>
#include <ranges>

/*
A data-structure for iterating through the indptr, i.e., sparse representation of data.
Indptr can be eg: [0, 3, 6, 7]
This means that:
objects 0, 1, 2 are coupled to index 0
objects 3, 4, 5 are coupled to index 1
objects 6 is coupled to index 2

Another intuitive way to look at this for python developers is like list of lists: [[0, 1, 2], [3, 4, 5], [6]].

DenseGroupedIdxVector is a vector of element to group. I.e., [0, 1, 1, 4] would denote that [[0], [1, 2], [], [], [3]]
The input, i.e., [0, 1, 3] should be strictly increasing

*/

namespace power_grid_model {

namespace detail {
template <std::ranges::viewable_range ElementGroups>
constexpr auto sparse_encode(ElementGroups&& element_groups, Idx num_groups) {
    IdxVector result(num_groups + 1);
    auto element_groups_view = std::views::all(std::forward<ElementGroups>(element_groups));
    auto next_group = std::begin(element_groups_view);
    for (auto const group : IdxRange{num_groups}) {
        next_group = std::upper_bound(next_group, std::end(element_groups_view), group);
        result[group + 1] = std::distance(std::begin(element_groups_view), next_group);
    }
    return result;
}

template <std::ranges::viewable_range IndPtr> constexpr auto sparse_decode(IndPtr&& indptr) {
    auto indptr_view = std::views::all(std::forward<IndPtr>(indptr));
    auto result = IdxVector(indptr_view.back());
    for (Idx const group : IdxRange{static_cast<Idx>(indptr_view.size()) - 1}) {
        std::fill(std::begin(result) + indptr_view[group], std::begin(result) + indptr_view[group + 1], group);
    }
    return result;
}

template <typename T, typename ElementType>
concept iterator_of = std::input_or_output_iterator<T> && requires(T const t) {
    { *t } -> std::convertible_to<std::remove_cvref_t<ElementType> const&>;
};

template <typename T, typename ElementType>
concept random_access_range_of = std::ranges::random_access_range<T> && requires(T const t) {
    { std::ranges::begin(t) } -> iterator_of<ElementType>;
    { std::ranges::end(t) } -> iterator_of<ElementType>;
};

template <typename T>
concept index_range_iterator = std::random_access_iterator<T> && requires(T const t) {
    typename T::iterator;
    { *t } -> random_access_range_of<Idx>;
};

template <typename T>
concept grouped_index_vector_type = std::default_initializable<T> && requires(T const t, Idx const idx) {
    typename T::iterator;

    { t.size() } -> std::same_as<Idx>;

    { t.begin() } -> index_range_iterator;
    { t.end() } -> index_range_iterator;
    { t.get_element_range(idx) } -> random_access_range_of<Idx>;

    { t.element_size() } -> std::same_as<Idx>;
    { t.get_group(idx) } -> std::same_as<Idx>;
};
} // namespace detail

template <typename T>
concept grouped_idx_vector_type = detail::grouped_index_vector_type<T>;

struct from_sparse_t {};
struct from_dense_t {};

constexpr auto from_sparse = from_sparse_t{};
constexpr auto from_dense = from_dense_t{};

class SparseGroupedIdxVector {
  private:
    class GroupIterator : public IteratorFacade {
      public:
        using iterator = GroupIterator;
        using const_iterator = std::add_const_t<GroupIterator>;
        using value_type = IdxRange const;
        using difference_type = Idx;
        using pointer = std::add_pointer_t<value_type>;
        using reference = std::add_lvalue_reference_t<value_type>;

        GroupIterator() : IteratorFacade{*this} {};
        explicit constexpr GroupIterator(IdxVector const& indptr, Idx group)
            : IteratorFacade{*this}, indptr_{&indptr}, group_{group} {}

        constexpr auto operator*() const -> reference {
            assert(indptr_ != nullptr);
            assert(0 <= group_);
            assert(group_ < static_cast<Idx>(indptr_->size() - 1));

            // delaying out-of-bounds checking until dereferencing while still returning a reference type requires
            // setting this here
            latest_value_ = value_type{(*indptr_)[group_], (*indptr_)[group_ + 1]};
            return latest_value_;
        }

        friend constexpr std::strong_ordering operator<=>(GroupIterator const& first, GroupIterator const& second) {
            assert(first.indptr_ == second.indptr_);
            return first.group_ <=> second.group_;
        }

        constexpr auto operator+=(difference_type n) -> std::add_lvalue_reference_t<GroupIterator> {
            group_ += n;
            return *this;
        }

        friend auto operator-(GroupIterator const& first, GroupIterator const& second) -> difference_type {
            assert(first.indptr_ == second.indptr_);
            return first.group_ - second.group_;
        }

      private:
        IdxVector const* indptr_{};
        Idx group_{};
        mutable std::remove_const_t<value_type>
            latest_value_{}; // making this mutable allows us to delay out-of-bounds checks until
                             // dereferencing instead of update methods. Note that the value will be
                             // invalidated at first update
    };

    constexpr auto group_iterator(Idx group) const { return GroupIterator{indptr_, group}; }

  public:
    using iterator = GroupIterator;

    constexpr auto size() const { return static_cast<Idx>(indptr_.size()) - 1; }
    constexpr auto begin() const { return group_iterator(0); }
    constexpr auto end() const { return group_iterator(size()); }
    constexpr auto get_element_range(Idx group) const { return *group_iterator(group); }

    constexpr auto element_size() const { return indptr_.back(); }
    constexpr auto get_group(Idx element) const -> Idx {
        assert(element < element_size());
        return std::distance(std::begin(indptr_), std::ranges::upper_bound(indptr_, element)) - 1;
    }

    constexpr SparseGroupedIdxVector() : indptr_{0} {};
    explicit constexpr SparseGroupedIdxVector(IdxVector sparse_group_elements)
        : indptr_{sparse_group_elements.empty() ? IdxVector{0} : std::move(sparse_group_elements)} {
        assert(size() >= 0);
        assert(element_size() >= 0);
        assert(std::ranges::is_sorted(indptr_));
    }
    constexpr SparseGroupedIdxVector(from_sparse_t /* tag */, IdxVector sparse_group_elements)
        : SparseGroupedIdxVector{std::move(sparse_group_elements)} {}
    constexpr SparseGroupedIdxVector(from_dense_t /* tag */, IdxVector const& dense_group_elements, Idx num_groups)
        : SparseGroupedIdxVector{detail::sparse_encode(dense_group_elements, num_groups)} {}

  private:
    IdxVector indptr_;
};

class DenseGroupedIdxVector {
  private:
    class GroupIterator : public IteratorFacade {
        friend class IteratorFacade; // to expose increment and decrement

      public:
        using iterator = GroupIterator;
        using const_iterator = std::add_const_t<GroupIterator>;
        using value_type = IdxRange const;
        using difference_type = Idx;
        using pointer = std::add_pointer_t<value_type>;
        using reference = std::add_lvalue_reference_t<value_type>;

        GroupIterator() : IteratorFacade{*this} {};
        explicit constexpr GroupIterator(IdxVector const& dense_vector, Idx group)
            : IteratorFacade{*this},
              dense_vector_{&dense_vector},
              group_{group},
              group_range_{std::ranges::equal_range(*dense_vector_, group)} {}

        constexpr auto operator*() const -> reference {
            assert(dense_vector_ != nullptr);

            // delaying out-of-bounds checking until dereferencing while still returning a reference type requires
            // setting this here
            latest_value_ =
                value_type{narrow_cast<Idx>(std::distance(std::cbegin(*dense_vector_), group_range_.begin())),
                           narrow_cast<Idx>(std::distance(std::cbegin(*dense_vector_), group_range_.end()))};
            return latest_value_;
        }

        friend constexpr std::strong_ordering operator<=>(GroupIterator const& first, GroupIterator const& second) {
            assert(first.dense_vector_ == second.dense_vector_);
            return first.group_ <=> second.group_;
        }

        constexpr auto operator+=(difference_type n) -> std::add_lvalue_reference_t<GroupIterator> {
            advance(n);
            return *this;
        }
        friend auto operator-(GroupIterator const& first, GroupIterator const& second) -> difference_type {
            assert(first.dense_vector_ == second.dense_vector_);
            return first.group_ - second.group_;
        }

      private:
        using group_iterator = IdxVector::const_iterator;

        IdxVector const* dense_vector_{};
        Idx group_{};
        std::ranges::subrange<group_iterator> group_range_;
        mutable std::remove_const_t<value_type>
            latest_value_{}; // making this mutable allows us to delay out-of-bounds checks until
                             // dereferencing instead of update methods. Note that the value will be
                             // invalidated at first update

        constexpr void increment() {
            ++group_;
            group_range_ = {group_range_.end(), std::find_if(group_range_.end(), std::cend(*dense_vector_),
                                                             [group = group_](Idx value) { return value > group; })};
        }
        constexpr void decrement() {
            --group_;
            group_range_ = {std::find_if(std::make_reverse_iterator(group_range_.begin()), std::crend(*dense_vector_),
                                         [group = group_](Idx value) { return value < group; })
                                .base(),
                            group_range_.begin()};
        }
        constexpr void advance(difference_type n) {
            auto const start = n > 0 ? group_range_.end() : std::cbegin(*dense_vector_);
            auto const stop = n < 0 ? group_range_.begin() : std::cend(*dense_vector_);

            group_ += n;
            group_range_ = std::ranges::equal_range(std::ranges::subrange{start, stop}, group_);
        }
    };

    constexpr auto group_iterator(Idx group) const { return GroupIterator{dense_vector_, group}; }

  public:
    using iterator = GroupIterator;

    constexpr auto size() const { return num_groups_; }
    constexpr auto begin() const { return group_iterator(Idx{}); }
    constexpr auto end() const { return group_iterator(size()); }

    constexpr auto element_size() const { return static_cast<Idx>(dense_vector_.size()); }
    constexpr auto get_group(Idx element) const { return dense_vector_[element]; }
    constexpr auto get_element_range(Idx group) const { return *group_iterator(group); }

    DenseGroupedIdxVector() = default;
    constexpr explicit DenseGroupedIdxVector(IdxVector dense_vector, Idx num_groups)
        : num_groups_{num_groups}, dense_vector_{std::move(dense_vector)} {
        assert(size() >= 0);
        assert(element_size() >= 0);
        assert(std::ranges::is_sorted(dense_vector_));
        assert(num_groups_ >= (dense_vector_.empty() ? 0 : dense_vector_.back()));
    }
    constexpr DenseGroupedIdxVector(from_sparse_t /* tag */, IdxVector const& sparse_group_elements)
        : DenseGroupedIdxVector{detail::sparse_decode(sparse_group_elements),
                                std::ranges::ssize(sparse_group_elements) - 1} {}
    constexpr DenseGroupedIdxVector(from_dense_t /* tag */, IdxVector dense_group_elements, Idx num_groups)
        : DenseGroupedIdxVector{std::move(dense_group_elements), num_groups} {}

  private:
    Idx num_groups_{};
    IdxVector dense_vector_;
};

static_assert(grouped_idx_vector_type<SparseGroupedIdxVector>);
static_assert(grouped_idx_vector_type<DenseGroupedIdxVector>);

inline auto enumerated_zip_sequence(grouped_idx_vector_type auto const& first,
                                    grouped_idx_vector_type auto const&... rest) {
    if constexpr (sizeof...(rest) > 0) {
        assert(((first.size() == rest.size()) && ...));
    }
    return std::views::zip(IdxRange{first.size()}, first, rest...);
}

} // namespace power_grid_model
