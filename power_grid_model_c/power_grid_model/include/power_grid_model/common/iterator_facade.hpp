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
concept iterator_facadeable_c = true;
//     requires(std::remove_const_t<T> t, std::add_const_t<T> ct, typename T::difference_type d) {
//         // typename T::value_type;
//         // std::integral<typename T::difference_type>;
//         // std::same_as<std::is_pointer<typename T::pointer>, std::true_type>;
//         // std::same_as<std::is_lvalue_reference<typename T::reference>, std::true_type>;
//         { *t } -> std::same_as<typename T::reference>;
//         { ct <=> ct } -> std::same_as<std::strong_ordering>;
//         { t.advance(d) };
//         { ct.distance_to(ct) } -> std::same_as<typename T::difference_type>;
//     };
} // namespace detail

class IteratorFacade {
  public:
    using iterator_category = std::random_access_iterator_tag;

    constexpr auto operator->(this auto&& self) -> decltype(auto) { return &(*std::forward<decltype(self)>(self)); }

    template <typename Self, typename Other>
        requires std::same_as<std::remove_cvref_t<Self>, std::remove_cvref_t<Other>>
    constexpr bool operator==(this Self const& self, Other const& other) {
        return (self <=> other) == std::strong_ordering::equivalent;
    }

    constexpr auto operator++(this auto& self) -> std::add_lvalue_reference_t<decltype(self)> {
        if constexpr (requires { self.increment(); }) {
            self.increment();
        } else {
            return (self += 1);
        }
        return self;
    }
    constexpr auto operator--(this auto& self) -> std::add_lvalue_reference_t<decltype(self)> {
        if constexpr (requires { self.decrement(); }) {
            self.decrement();
        } else {
            return (self += -1);
        }
        return self;
    }
    template <typename Self>
    constexpr auto operator++(this Self& self, std::integral auto /*idx*/) -> std::remove_cvref_t<Self> {
        using Result = std::remove_cvref_t<Self>;
        Result result{self};
        ++self;
        return result;
    }
    template <typename Self>
    constexpr auto operator--(this Self& self, std::integral auto /*idx*/) -> std::remove_cvref_t<Self> {
        using Result = std::remove_cvref_t<Self>;
        Result result{self};
        --self;
        return result;
    }
    constexpr auto operator+=(this auto& self,
                              std::integral auto offset) -> std::add_lvalue_reference_t<decltype(self)> {
        self.advance(offset);
        return self;
    }
    constexpr auto operator-=(this auto& self, std::integral auto idx) -> std::add_lvalue_reference_t<decltype(self)> {
        return (self += (-idx));
    }

    template <typename Self>
    constexpr auto operator+(this Self&& self, std::integral auto offset) -> std::remove_cvref_t<Self> {
        using Result = std::remove_cvref_t<Self>;
        Result result{std::forward<Self>(self)};
        result += offset;
        return result;
    }
    template <typename Self>
        requires std::derived_from<std::remove_cvref_t<Self>, IteratorFacade> &&
                     detail::iterator_facadeable_c<std::remove_cvref_t<Self>>
    friend constexpr auto operator+(std::integral auto offset, Self&& self) -> std::remove_cvref_t<Self> {
        return std::forward<Self>(self) + offset;
    }
    template <typename Self>
    constexpr auto operator-(this Self&& self, std::integral auto idx) -> std::remove_cvref_t<Self> {
        return (std::forward<Self>(self)) + (-idx);
    }
    template <typename Self, typename Other>
        requires std::same_as<std::remove_cvref_t<Self>, std::remove_cvref_t<Other>>
    constexpr auto operator-(this Self&& self, Other&& other) {
        return std::forward<Other>(other).distance_to(std::forward<Self>(self));
    }

    template <typename Self>
    constexpr auto operator[](this Self const& self, typename Self::difference_type idx) -> decltype(auto) {
        return *(self + idx);
    }

    // delete default, constructors of non-derived types and non-iterator-facadeable types to prevent instantiation from
    // non-derived classes
    IteratorFacade() = delete;
    template <typename Self>
        requires(
            std::derived_from<std::remove_cvref_t<Self>, IteratorFacade>) // && detail::iterator_facadeable_c<Self>)
    explicit IteratorFacade(Self&& /*self*/){};
};

} // namespace power_grid_model
