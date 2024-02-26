// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common.hpp"
#include "counting_iterator.hpp"
#include "typing.hpp"

#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/range.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <boost/range/counting_range.hpp>

#include <ranges>

/*
A data-structure for iterating through the indptr, ie. sparse representation of data.
Indptr can be eg: [0, 3, 6, 7]
This means that:
objects 0, 1, 2 are coupled to index 0
objects 3, 4, 5 are coupled to index 1
objects 6 is coupled to index 2

Another intuitive way to look at this for python developers is like list of lists: [[0, 1, 2], [3, 4, 5], [6]].

DenseGroupedIdxVector is a vector of element to group. ie. [0, 1, 1, 4] would denote that [[0], [1, 2], [], [], [3]]
The input, ie. [0, 1, 3] should be strictly increasing

*/

namespace power_grid_model {

using IdxRange = boost::iterator_range<IdxCount>;

namespace detail {

inline auto sparse_encode(IdxVector const& element_groups, Idx num_groups) {
    IdxVector result(num_groups + 1);
    auto next_group = std::begin(element_groups);
    for (auto const group : boost::counting_range(Idx{0}, num_groups)) {
        next_group = std::upper_bound(next_group, std::end(element_groups), group);
        result[group + 1] = std::distance(std::begin(element_groups), next_group);
    }
    return result;
}

inline auto sparse_decode(IdxVector const& indptr) {
    auto result = IdxVector(indptr.back());
    for (Idx const group : boost::counting_range(Idx{0}, static_cast<Idx>(indptr.size()) - 1)) {
        std::fill(std::begin(result) + indptr[group], std::begin(result) + indptr[group + 1], group);
    }
    return result;
}

// TODO(mgovers): replace the below relevant subset here ourselves with the STD equivalent when we have std::ranges.
// boost::counting_iterator does not satisfy all requirements std::*_iterator concepts:
static_assert(!std::random_access_iterator<IdxCount>);
// we have to declare the relevant subset here ourselves.
template <typename T, typename ElementType>
concept iterator_like = requires(T const t) {
                            { *t } -> std::convertible_to<std::remove_cvref_t<ElementType> const&>;
                        };

template <typename T, typename ElementType>
concept random_access_iterator_like =
    std::regular<T> && iterator_like<T, ElementType> && std::totally_ordered<T> && requires(T t, Idx n) {
                                                                                       { t++ } -> std::same_as<T>;
                                                                                       { t-- } -> std::same_as<T>;
                                                                                       { ++t } -> std::same_as<T&>;
                                                                                       { --t } -> std::same_as<T&>;

                                                                                       { t + n } -> std::same_as<T>;
                                                                                       { t - n } -> std::same_as<T>;
                                                                                       { t += n } -> std::same_as<T&>;
                                                                                       { t -= n } -> std::same_as<T&>;
                                                                                   };

template <typename T, typename ElementType>
concept random_access_iterable_like = requires(T const t) {
                                          { t.begin() } -> random_access_iterator_like<ElementType>;
                                          { t.end() } -> random_access_iterator_like<ElementType>;
                                      };

template <typename T>
concept index_range_iterator =
    random_access_iterator_like<T, typename T::iterator> && requires(T const t) {
                                                                typename T::iterator;
                                                                { *t } -> random_access_iterable_like<Idx>;
                                                            };

template <typename T>
concept grouped_index_vector_type = std::default_initializable<T> && requires(T const t, Idx const idx) {
                                                                         typename T::iterator;

                                                                         { t.size() } -> std::same_as<Idx>;

                                                                         { t.begin() } -> index_range_iterator;
                                                                         { t.end() } -> index_range_iterator;
                                                                         {
                                                                             t.get_element_range(idx)
                                                                             } -> random_access_iterable_like<Idx>;

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
    class GroupIterator
        : public boost::iterator_facade<GroupIterator, Idx, boost::random_access_traversal_tag, IdxRange, Idx> {
      public:
        using iterator = IdxRange;

        GroupIterator() = default;
        explicit constexpr GroupIterator(IdxVector const& indptr, Idx group) : indptr_{&indptr}, group_{group} {}

      private:
        IdxVector const* indptr_{};
        Idx group_{};

        friend class boost::iterator_core_access;

        auto dereference() const -> iterator {
            assert(indptr_ != nullptr);
            return boost::counting_range((*indptr_)[group_], (*indptr_)[group_ + 1]);
        }
        constexpr auto equal(GroupIterator const& other) const { return group_ == other.group_; }
        constexpr auto distance_to(GroupIterator const& other) const { return other.group_ - group_; }

        constexpr void increment() { ++group_; }
        constexpr void decrement() { --group_; }
        constexpr void advance(Idx n) { group_ += n; }
    };

    auto group_iterator(Idx group) const { return GroupIterator{indptr_, group}; }

  public:
    using iterator = GroupIterator;

    auto size() const { return static_cast<Idx>(indptr_.size()) - 1; }
    auto begin() const { return group_iterator(0); }
    auto end() const { return group_iterator(size()); }
    auto get_element_range(Idx group) const { return *group_iterator(group); }

    auto element_size() const { return indptr_.back(); }
    auto get_group(Idx element) const -> Idx {
        assert(element < element_size());
        return std::distance(std::begin(indptr_), std::ranges::upper_bound(indptr_, element)) - 1;
    }

    SparseGroupedIdxVector() : indptr_{0} {};
    explicit SparseGroupedIdxVector(IdxVector sparse_group_elements)
        : indptr_{sparse_group_elements.empty() ? IdxVector{0} : std::move(sparse_group_elements)} {
        assert(size() >= 0);
        assert(element_size() >= 0);
        assert(std::is_sorted(std::begin(indptr_), std::end(indptr_)));
    }
    SparseGroupedIdxVector(from_sparse_t /* tag */, IdxVector sparse_group_elements)
        : SparseGroupedIdxVector{std::move(sparse_group_elements)} {}
    SparseGroupedIdxVector(from_dense_t /* tag */, IdxVector const& dense_group_elements, Idx num_groups)
        : SparseGroupedIdxVector{detail::sparse_encode(dense_group_elements, num_groups)} {}

  private:
    IdxVector indptr_;
};

class DenseGroupedIdxVector {
  private:
    class GroupIterator
        : public boost::iterator_facade<GroupIterator, Idx, boost::random_access_traversal_tag, IdxRange, Idx> {
      public:
        using iterator = IdxRange;

        GroupIterator() = default;
        explicit constexpr GroupIterator(IdxVector const& dense_vector, Idx group)
            : dense_vector_{&dense_vector},
              group_{group},
              group_range_{std::equal_range(std::cbegin(*dense_vector_), std::cend(*dense_vector_), group)} {}

      private:
        using group_iterator = IdxVector::const_iterator;

        IdxVector const* dense_vector_{};
        Idx group_{};
        std::pair<group_iterator, group_iterator> group_range_{};

        friend class boost::iterator_core_access;

        auto dereference() const -> iterator {
            assert(dense_vector_ != nullptr);
            return boost::counting_range(
                narrow_cast<Idx>(std::distance(std::cbegin(*dense_vector_), group_range_.first)),
                narrow_cast<Idx>(std::distance(std::cbegin(*dense_vector_), group_range_.second)));
        }
        constexpr auto equal(GroupIterator const& other) const { return group_ == other.group_; }
        constexpr auto distance_to(GroupIterator const& other) const { return other.group_ - group_; }

        constexpr void increment() {
            ++group_;
            group_range_ = std::make_pair(group_range_.second,
                                          std::find_if(group_range_.second, std::cend(*dense_vector_),
                                                       [group = group_](Idx value) { return value > group; }));
        }
        constexpr void decrement() {
            --group_;
            group_range_ =
                std::make_pair(std::find_if(std::make_reverse_iterator(group_range_.first), std::crend(*dense_vector_),
                                            [group = group_](Idx value) { return value < group; })
                                   .base(),
                               group_range_.first);
        }
        constexpr void advance(Idx n) {
            auto const start = n > 0 ? group_range_.second : std::cbegin(*dense_vector_);
            auto const stop = n < 0 ? group_range_.first : std::cend(*dense_vector_);

            group_ += n;
            group_range_ = std::equal_range(start, stop, group_);
        }
    };

    auto group_iterator(Idx group) const { return GroupIterator{dense_vector_, group}; }

  public:
    using iterator = GroupIterator;

    auto size() const { return num_groups_; }
    auto begin() const { return group_iterator(Idx{}); }
    auto end() const { return group_iterator(size()); }

    auto element_size() const { return static_cast<Idx>(dense_vector_.size()); }
    auto get_group(Idx element) const { return dense_vector_[element]; }
    auto get_element_range(Idx group) const { return *group_iterator(group); }

    DenseGroupedIdxVector() = default;
    explicit DenseGroupedIdxVector(IdxVector dense_vector, Idx num_groups)
        : num_groups_{num_groups}, dense_vector_{std::move(dense_vector)} {
        assert(size() >= 0);
        assert(element_size() >= 0);
        assert(std::is_sorted(std::begin(dense_vector_), std::end(dense_vector_)));
        assert(num_groups_ >= (dense_vector_.empty() ? 0 : dense_vector_.back()));
    }
    DenseGroupedIdxVector(from_sparse_t /* tag */, IdxVector const& sparse_group_elements)
        : DenseGroupedIdxVector{detail::sparse_decode(sparse_group_elements),
                                static_cast<Idx>(sparse_group_elements.size()) - 1} {}
    DenseGroupedIdxVector(from_dense_t /* tag */, IdxVector dense_group_elements, Idx num_groups)
        : DenseGroupedIdxVector{std::move(dense_group_elements), num_groups} {}

  private:
    Idx num_groups_{};
    IdxVector dense_vector_;
};

static_assert(grouped_idx_vector_type<SparseGroupedIdxVector>);
static_assert(grouped_idx_vector_type<DenseGroupedIdxVector>);

inline auto enumerated_zip_sequence(grouped_idx_vector_type auto const& first,
                                    grouped_idx_vector_type auto const&... rest) {
    assert(((first.size() == rest.size()) && ...));

    auto const indices = boost::counting_range(Idx{}, first.size());

    auto const zip_begin =
        boost::make_zip_iterator(boost::make_tuple(std::begin(indices), std::begin(first), std::begin(rest)...));
    auto const zip_end =
        boost::make_zip_iterator(boost::make_tuple(std::end(indices), std::end(first), std::end(rest)...));
    return boost::make_iterator_range(zip_begin, zip_end);
}

} // namespace power_grid_model
