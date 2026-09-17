// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <atomic>
#include <concepts>
#include <type_traits>

namespace power_grid_model::common::atomic {
template <typename T>
    requires(std::is_trivially_copyable_v<T> && std::copy_constructible<T> && std::move_constructible<T> &&
             std::is_copy_assignable_v<T> && std::is_move_assignable_v<T> && std::same_as<T, std::remove_cv_t<T>>)
class CopyableAtomic : public std::atomic<T> { // NOSONAR(S3642)
  private:
    static constexpr std::memory_order copy_order = std::memory_order::seq_cst;

  public:
    CopyableAtomic() noexcept = default;
    template <typename... Args>
        requires(std::constructible_from<T, Args...> && !std::same_as<std::remove_cvref_t<T>, CopyableAtomic<T>>)
    CopyableAtomic(Args&&... args) noexcept : std::atomic<T>{std::forward<Args>(args)...} {}
    CopyableAtomic(CopyableAtomic const& other) noexcept : std::atomic<T>{other.load(copy_order)} {} // NOSONAR(S3642)
    CopyableAtomic(CopyableAtomic&& other) noexcept = default;
    CopyableAtomic& operator=(CopyableAtomic const& other) noexcept {
        if (this != &other) {
            this->store(other.load(copy_order));
        }
        return *this;
    }
    CopyableAtomic& operator=(CopyableAtomic&& other) noexcept = default;
    ~CopyableAtomic() noexcept = default;
};
} // namespace power_grid_model::common::atomic
