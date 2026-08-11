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
    Underlying _underlying;
    View _view;

  public:
    maybe_owning_view() = default;

    constexpr maybe_owning_view(Underlying&& underlying) : _underlying(std::move(underlying)), _view{_underlying} {}
    constexpr maybe_owning_view(View view) : _underlying{/*default is empty*/}, _view{std::move(view)} {}

    maybe_owning_view(maybe_owning_view&&) = default;
    constexpr maybe_owning_view(maybe_owning_view const&) = delete;
    maybe_owning_view& operator=(maybe_owning_view&&) = default;
    constexpr maybe_owning_view& operator=(maybe_owning_view const&) = delete;
    ~maybe_owning_view() = default;

    constexpr maybe_owning_view& operator=(Underlying&& underlying) {
        _underlying = std::move(underlying);
        _view = _underlying;
        return *this;
    }
    constexpr maybe_owning_view& operator=(View view) {
        _underlying = {/*default is empty*/};
        _view = std::move(view);
        return *this;
    }

    constexpr operator View() const { return _view; }

    constexpr auto begin() { return std::ranges::begin(_view); }
    constexpr auto end() { return std::ranges::end(_view); }
    constexpr auto begin() const
        requires std::ranges::range<Underlying const>
    {
        return std::ranges::begin(_view);
    }
    constexpr auto end() const
        requires std::ranges::range<Underlying const>
    {
        return std::ranges::end(_view);
    }
    constexpr bool empty() const
        requires requires { std::ranges::empty(_view); }
    {
        return std::ranges::empty(_view);
    }
    constexpr auto size() const
        requires std::ranges::sized_range<View>
    {
        return std::ranges::size(_view);
    }
    constexpr auto& operator[](std::integral auto idx)
        requires std::ranges::random_access_range<View>
    {
        return _view[idx];
    }
    constexpr auto const& operator[](std::integral auto idx) const
        requires std::ranges::random_access_range<View>
    {
        return _view[idx];
    }
    constexpr auto* data()
        requires std::ranges::contiguous_range<View>
    {
        return _view.data();
    }
    constexpr auto const* data() const
        requires std::ranges::contiguous_range<View>
    {
        return _view.data();
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
