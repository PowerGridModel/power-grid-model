// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common.hpp"

#include <concepts>
#include <ranges>
#include <utility>

namespace power_grid_model {
namespace detail {
template <typename T>
concept iterator_facadeable_c =
    std::integral<typename T::difference_type> && std::is_pointer_v<typename T::pointer> &&
    std::is_lvalue_reference_v<typename T::reference> &&
    requires(T t, std::add_const_t<T> ct, std::add_const_t<T> ct2, typename T::difference_type d) {
        typename T::value_type;
        { *t } -> std::same_as<typename T::reference>;
        { &*t } -> std::same_as<typename T::pointer>;
        { ct <=> ct2 } -> std::same_as<std::strong_ordering>;
        { t.advance(d) };
        { ct.distance_to(ct) } -> std::same_as<typename T::difference_type>;
    };
} // namespace detail

class IteratorFacade {
  public:
    using iterator_category = std::random_access_iterator_tag;

    template <typename Self> constexpr decltype(auto) operator->(this Self&& self) {
        return &(*std::forward<Self>(self));
    }

    template <typename Self, typename Other>
        requires std::same_as<std::remove_cvref_t<Self>, std::remove_cvref_t<Other>>
    constexpr bool operator==(this Self const& self, Other const& other) {
        return (self <=> other) == std::strong_ordering::equivalent;
    }

    template <typename Self> constexpr std::add_lvalue_reference_t<Self> operator++(this Self& self) {
        if constexpr (requires { self.increment(); }) {
            self.increment();
        } else {
            return (self += 1);
        }
        return self;
    }
    template <typename Self> constexpr std::add_lvalue_reference_t<Self> operator--(this Self& self) {
        if constexpr (requires { self.decrement(); }) {
            self.decrement();
        } else {
            return (self += -1);
        }
        return self;
    }
    template <typename Self>
    constexpr std::remove_cvref_t<Self> operator++(this Self& self, std::integral auto /*idx*/) {
        using Result = std::remove_cvref_t<Self>;
        Result result{self};
        ++self;
        return result;
    }
    template <typename Self>
    constexpr std::remove_cvref_t<Self> operator--(this Self& self, std::integral auto /*idx*/) {
        using Result = std::remove_cvref_t<Self>;
        Result result{self};
        --self;
        return result;
    }
    template <typename Self>
    constexpr std::add_lvalue_reference_t<Self> operator+=(this Self& self, std::integral auto offset) {
        self.advance(offset);
        return self;
    }
    template <typename Self>
    constexpr std::add_lvalue_reference_t<Self> operator-=(this Self& self, std::integral auto idx) {
        return (self += (-idx));
    }

    template <typename Self>
    constexpr std::remove_cvref_t<Self> operator+(this Self&& self, std::integral auto offset) {
        using Result = std::remove_cvref_t<Self>;
        Result result{std::forward<Self>(self)};
        result += offset;
        return result;
    }
    template <typename Self>
        requires std::derived_from<std::remove_cvref_t<Self>, IteratorFacade> &&
                 detail::iterator_facadeable_c<std::remove_cvref_t<Self>>
    friend constexpr std::remove_cvref_t<Self> operator+(std::integral auto offset, Self&& self) {
        return std::forward<Self>(self) + offset;
    }
    template <typename Self>
        requires std::derived_from<std::remove_cvref_t<Self>, IteratorFacade> &&
                 detail::iterator_facadeable_c<std::remove_cvref_t<Self>>
    friend constexpr std::remove_cvref_t<Self> operator-(Self&& self, std::integral auto idx) {
        return (std::forward<Self>(self)) + (-idx);
    }
    template <typename Self, typename Other>
        requires std::derived_from<std::remove_cvref_t<Self>, IteratorFacade> &&
                 detail::iterator_facadeable_c<std::remove_cvref_t<Self>> &&
                 std::same_as<std::remove_cvref_t<Self>, std::remove_cvref_t<Other>>
    friend constexpr auto operator-(Self&& self, Other&& other) {
        return std::forward<Other>(other).distance_to(std::forward<Self>(self));
    }

    template <typename Self>
    constexpr decltype(auto) operator[](this Self const& self, typename Self::difference_type idx) {
        return *(self + idx);
    }

    // prevent construction by non-derived and non-iterator-facadeable types
    IteratorFacade() = delete;
    template <typename Self> constexpr explicit IteratorFacade(Self& /*self*/) {
        // cannot be done using constraints because the type is not fully instantiated yet when the compiler
        // instantiates the constructor. Note that this is different from the other methods because those are only
        // instantiated when used.
        static_assert(std::derived_from<std::remove_cvref_t<Self>, IteratorFacade>);
        static_assert(detail::iterator_facadeable_c<std::remove_cvref_t<Self>>);
    };
};

} // namespace power_grid_model
