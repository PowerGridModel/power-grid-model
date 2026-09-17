// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <atomic>
#include <concepts>

namespace power_grid_model::common::atomic {
template <typename T>
    requires(std::is_trivially_copyable<T>::value && std::copy_constructible<T> && std::move_constructible<T> &&
             std::is_copy_assignable<T>::value && std::is_move_assignable<T>::value &&
             std::same_as<T, typename std::remove_cv<T>::type>)
class CopyableAtomic : public std::atomic<T> {
  private:
    static constexpr std::memory_order copy_order = std::memory_order::seq_cst;

  public:
    CopyableAtomic() noexcept = default;
    template <typename... Args>
    CopyableAtomic(Args&&... args) noexcept : std::atomic<T>{std::forward<Args>(args)...} {}
    CopyableAtomic(CopyableAtomic const& other) noexcept : std::atomic<T>{other.load(copy_order)} {}
    CopyableAtomic(CopyableAtomic&& other) noexcept = default;
    CopyableAtomic& operator=(CopyableAtomic const& other) noexcept {
        this->store(other.load(copy_order));
        return *this;
    }
    CopyableAtomic& operator=(CopyableAtomic&& other) noexcept = default;
    ~CopyableAtomic() noexcept = default;
};
} // namespace power_grid_model::common::atomic
