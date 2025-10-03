// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/common/iterator_facade.hpp>
#include <power_grid_model/common/counting_iterator.hpp>

#include <doctest/doctest.h>

namespace {
using power_grid_model::Idx;
using power_grid_model::IdxRange;
using power_grid_model::IdxVector;
using power_grid_model::IteratorFacade;
using power_grid_model::detail::iterator_facadeable_c;

template <typename UnderlyingType> class BaseTestIterator : public IteratorFacade {
    friend class IteratorFacade;

  public:
    using underlying = UnderlyingType;

    using value_type = underlying::value_type;
    using difference_type = underlying::difference_type;
    using pointer = underlying::pointer;
    using reference = underlying::reference;

    constexpr BaseTestIterator(underlying it) : IteratorFacade{*this}, it_{it} {}

    constexpr auto operator*() -> reference { return *it_; }
    constexpr auto operator*() const -> std::add_lvalue_reference_t<std::add_const_t<value_type>> { return *it_; }

    friend constexpr auto operator<=>(BaseTestIterator const& first, BaseTestIterator const& second)
        -> std::strong_ordering {
        return *first.it_ <=> *second.it_;
    }

    constexpr auto distance_to(BaseTestIterator const& other) const -> difference_type { return other.it_ - it_; }
    constexpr void advance(difference_type n) { it_ += n; }

  private:
    underlying it_;
};

using TestIdxVectorIterator = BaseTestIterator<typename IdxVector::iterator>;
using TestIdxVectorConstIterator = BaseTestIterator<typename IdxVector::const_iterator>;

static_assert(iterator_facadeable_c<TestIdxVectorIterator>);
static_assert(iterator_facadeable_c<TestIdxVectorConstIterator>);
static_assert(std::constructible_from<IteratorFacade, TestIdxVectorIterator>);
static_assert(std::constructible_from<IteratorFacade, TestIdxVectorConstIterator>);
} // namespace

TEST_CASE_TEMPLATE("Test IteratorFacade", T, TestIdxVectorIterator, TestIdxVectorConstIterator) {
    using TestIterator = T;

    auto vec = IdxRange{40} | std::ranges::to<IdxVector>();

    SUBCASE("Basic operations") {
        auto it = TestIterator{vec.begin() + 5};
        CHECK(*it == 5);
        CHECK(*(++it) == 6);
        CHECK(*it++ == 6);
        CHECK(*it == 7);
        CHECK(*(--it) == 6);
        CHECK(*it-- == 6);
        CHECK(*it == 5);
        it += 3;
        CHECK(*it == 8);
        it -= 2;
        CHECK(*it == 6);
        it = it + 4;
        CHECK(*it == 10);
        it = 20 + it;
        CHECK(*it == 30);
        it = it - 5;
        CHECK(*it == 25);

        auto const it2 = TestIterator{vec.begin() + 25};
        CHECK(it == it2);
        CHECK((it <=> it2) == std::strong_ordering::equivalent);
        auto const it3 = TestIterator{vec.begin() + 30};
        CHECK((it <=> it3) == std::strong_ordering::less);
        CHECK((it3 <=> it) == std::strong_ordering::greater);

        auto dist = it2 - it;
        CHECK(dist == 0);

        auto const it4 = TestIterator{vec.begin() + 15};
        dist = it2 - it4;
        CHECK(dist == 10);
    }
    if constexpr (std::is_same_v<TestIterator, TestIdxVectorIterator>) {
        SUBCASE("Mutate elements") {
            auto it = TestIterator{vec.begin() + 5};
            *it = 42;
            CHECK(*it == 42);
            CHECK(vec[5] == 42);
        }
    }
}
