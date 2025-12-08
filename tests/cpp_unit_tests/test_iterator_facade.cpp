// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/common/counting_iterator.hpp>
#include <power_grid_model/common/iterator_facade.hpp>

#include <doctest/doctest.h>

#include <optional>

namespace {
using power_grid_model::Idx;
using power_grid_model::IdxRange;
using power_grid_model::IdxVector;
using power_grid_model::IntS;
using power_grid_model::IteratorFacade;
using power_grid_model::detail::iterator_facadeable_c;

enum class IteratorFacadeableCalls : IntS {
    none = 0,
    dereference = 1,
    advance = 2,
    distance_to = 3,
    increment = 4,
    decrement = 5,
};

struct without_increment_decrement_t {};
struct with_increment_decrement_t {};

template <typename T>
concept advance_type_c = std::same_as<T, without_increment_decrement_t> || std::same_as<T, with_increment_decrement_t>;

template <advance_type_c advance_type_t, typename UnderlyingType> class BaseTestIterator : public IteratorFacade {
    friend class IteratorFacade;

  public:
    using underlying = UnderlyingType;

    using value_type = underlying::value_type;
    using difference_type = underlying::difference_type;
    using pointer = underlying::pointer;
    using reference = underlying::reference;

    static constexpr auto increment_style = std::same_as<advance_type_t, with_increment_decrement_t>
                                                ? IteratorFacadeableCalls::increment
                                                : IteratorFacadeableCalls::advance;
    static constexpr auto decrement_style = std::same_as<advance_type_t, with_increment_decrement_t>
                                                ? IteratorFacadeableCalls::decrement
                                                : IteratorFacadeableCalls::advance;

    constexpr BaseTestIterator(std::remove_cvref_t<underlying> it) : IteratorFacade{*this}, it_{std::move(it)} {}

    constexpr auto operator*() -> reference {
        last_call_ = IteratorFacadeableCalls::dereference;
        return *it_;
    }
    constexpr auto operator*() const -> std::add_lvalue_reference_t<std::add_const_t<value_type>> {
        last_call_ = IteratorFacadeableCalls::dereference;
        return *it_;
    }

    friend constexpr auto operator<=>(BaseTestIterator const& first,
                                      BaseTestIterator const& second) -> std::strong_ordering {
        first.last_call_ = IteratorFacadeableCalls::distance_to;
        return *first.it_ <=> *second.it_;
    }

    constexpr decltype(auto) operator+=(difference_type n) {
        last_call_ = IteratorFacadeableCalls::advance;
        it_ += n;
        return *this;
    }
    friend constexpr auto operator-(BaseTestIterator const& first, BaseTestIterator const& second) -> difference_type {
        first.last_call_ = IteratorFacadeableCalls::distance_to;
        return first.it_ - second.it_;
    }

    constexpr std::optional<IteratorFacadeableCalls> const& last_call() const { return last_call_; }
    constexpr void reset() { last_call_ = std::nullopt; }

  private:
    underlying it_;
    mutable std::optional<IteratorFacadeableCalls> last_call_; // mutable to allow const methods to set it

    constexpr void increment()
        requires std::same_as<advance_type_t, with_increment_decrement_t>
    {
        last_call_ = IteratorFacadeableCalls::increment;
        ++it_;
    }
    constexpr void decrement()
        requires std::same_as<advance_type_t, with_increment_decrement_t>
    {
        last_call_ = IteratorFacadeableCalls::decrement;
        --it_;
    }
};

using TestIdxVectorIterator = BaseTestIterator<without_increment_decrement_t, typename IdxVector::iterator>;
using TestIdxVectorConstIterator = BaseTestIterator<without_increment_decrement_t, typename IdxVector::const_iterator>;
using TestIdxVectorIteratorWithIncDec = BaseTestIterator<with_increment_decrement_t, typename IdxVector::iterator>;
using TestIdxVectorConstIteratorWithIncDec =
    BaseTestIterator<with_increment_decrement_t, typename IdxVector::const_iterator>;

static_assert(iterator_facadeable_c<TestIdxVectorIterator>);
static_assert(iterator_facadeable_c<TestIdxVectorConstIterator>);
static_assert(iterator_facadeable_c<TestIdxVectorIteratorWithIncDec>);
static_assert(iterator_facadeable_c<TestIdxVectorConstIteratorWithIncDec>);

static_assert(std::constructible_from<IteratorFacade, TestIdxVectorIterator>);
static_assert(std::constructible_from<IteratorFacade, TestIdxVectorConstIterator>);
static_assert(std::constructible_from<IteratorFacade, TestIdxVectorIteratorWithIncDec>);
static_assert(std::constructible_from<IteratorFacade, TestIdxVectorConstIteratorWithIncDec>);

} // namespace

TYPE_TO_STRING_AS("TestIdxVectorIterator", TestIdxVectorIterator);
TYPE_TO_STRING_AS("TestIdxVectorConstIterator", TestIdxVectorConstIterator);
TYPE_TO_STRING_AS("TestIdxVectorIteratorWithIncDec", TestIdxVectorIteratorWithIncDec);
TYPE_TO_STRING_AS("TestIdxVectorConstIteratorWithIncDec", TestIdxVectorConstIteratorWithIncDec);

TEST_CASE_TEMPLATE("Test IteratorFacade", T, TestIdxVectorIterator, TestIdxVectorConstIterator,
                   TestIdxVectorIteratorWithIncDec, TestIdxVectorConstIteratorWithIncDec) {
    using TestIterator = T;

    auto vec = IdxRange{40} | std::ranges::to<IdxVector>();

    SUBCASE("Basic operations") {
        auto it = TestIterator{vec.begin() + 5};
        CHECK(*it == 5);
        CHECK(*(++it) == 6);
        CHECK(*(it++) == 6);
        CHECK(*it == 7);
        CHECK(*(--it) == 6);
        CHECK(*(it--) == 6);
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
        dist = it4 - it2;
        CHECK(dist == -10);
    }
    if constexpr (std::is_same_v<TestIterator, TestIdxVectorIterator>) {
        SUBCASE("Mutate elements") {
            auto it = TestIterator{vec.begin() + 5};
            *it = 42;
            CHECK(*it == 42);
            CHECK(vec[5] == 42);
        }
    }
    SUBCASE("Incremented/Decremented calls") {
        auto it = TestIterator{vec.begin() + 5};
        REQUIRE(!it.last_call().has_value());

        ++it;
        REQUIRE(it.last_call().has_value());
        CHECK(it.last_call().value() == TestIterator::increment_style);
        it.reset();
        REQUIRE(!it.last_call().has_value());

        it++;
        REQUIRE(it.last_call().has_value());
        CHECK(it.last_call().value() == TestIterator::increment_style);
        it.reset();
        REQUIRE(!it.last_call().has_value());

        --it;
        REQUIRE(it.last_call().has_value());
        CHECK(it.last_call().value() == TestIterator::decrement_style);
        it.reset();
        REQUIRE(!it.last_call().has_value());

        it--;
        REQUIRE(it.last_call().has_value());
        CHECK(it.last_call().value() == TestIterator::decrement_style);
        it.reset();
    }
}
