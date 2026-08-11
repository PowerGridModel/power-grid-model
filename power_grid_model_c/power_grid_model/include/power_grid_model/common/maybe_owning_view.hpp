// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common.hpp"

#include <concepts>
#include <ranges>
#include <variant>

namespace power_grid_model {
template <std::ranges::range Underlying, std::ranges::view View>
    requires std::same_as<std::ranges::range_value_t<Underlying>, std::ranges::range_value_t<View>> &&
             std::movable<Underlying> && std::default_initializable<Underlying>
class maybe_owning_view : public std::ranges::view_interface<maybe_owning_view<Underlying, View>> {
    Underlying underlying_;
    View view_;

  public:
    maybe_owning_view() = default;

    constexpr maybe_owning_view(Underlying&& underlying) : underlying_(std::move(underlying)), view_{underlying_} {}
    constexpr maybe_owning_view(View view) : underlying_{/*default is empty*/}, view_{std::move(view)} {}

    maybe_owning_view(maybe_owning_view&&) noexcept = default;
    constexpr maybe_owning_view(maybe_owning_view const&) = delete;
    maybe_owning_view& operator=(maybe_owning_view&&) noexcept = default;
    constexpr maybe_owning_view& operator=(maybe_owning_view const&) = delete;
    ~maybe_owning_view() = default;

    constexpr maybe_owning_view& operator=(Underlying&& underlying) {
        underlying_ = std::move(underlying);
        view_ = underlying_;
        return *this;
    }
    constexpr maybe_owning_view& operator=(View view) {
        underlying_ = {/*default is empty*/};
        view_ = std::move(view);
        return *this;
    }

    constexpr operator View() const { return view_; }

    constexpr auto begin() { return std::ranges::begin(view_); }
    constexpr auto end() { return std::ranges::end(view_); }
    constexpr auto begin() const
        requires std::ranges::range<Underlying const>
    {
        return std::ranges::begin(view_);
    }
    constexpr auto end() const
        requires std::ranges::range<Underlying const>
    {
        return std::ranges::end(view_);
    }
    constexpr bool empty() const
        requires requires { std::ranges::empty(view_); }
    {
        return std::ranges::empty(view_);
    }
    constexpr auto size() const
        requires std::ranges::sized_range<View>
    {
        return std::ranges::size(view_);
    }
    constexpr auto& operator[](std::integral auto idx)
        requires std::ranges::random_access_range<View>
    {
        return view_[idx];
    }
    constexpr auto const& operator[](std::integral auto idx) const
        requires std::ranges::random_access_range<View>
    {
        return view_[idx];
    }
    constexpr auto* data()
        requires std::ranges::contiguous_range<View>
    {
        return view_.data();
    }
    constexpr auto const* data() const
        requires std::ranges::contiguous_range<View>
    {
        return view_.data();
    }
};

template <class R, std::ranges::view V>
    requires std::ranges::view<maybe_owning_view<R, V>>
struct detail::is_owning_view<maybe_owning_view<R, V>> : std::true_type {}; // customization point

template <typename T> using MaybeOwningVector = maybe_owning_view<std::vector<T>, std::span<T>>;
template <typename T>
using MaybeOwningConstVector = maybe_owning_view<std::vector<std::remove_const_t<T>>, std::span<T const>>;

static_assert(std::ranges::viewable_range<MaybeOwningVector<Idx>>);
static_assert(std::ranges::viewable_range<MaybeOwningConstVector<Idx>>);
} // namespace power_grid_model
