// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CONTAINER_HPP
#define POWER_GRID_MODEL_CONTAINER_HPP

// container for multiple components

#include "exception.hpp"
#include "power_grid_model.hpp"

#include <boost/iterator/iterator_facade.hpp>

#include <functional>
#include <memory>
#include <unordered_map>

namespace power_grid_model {

namespace container_impl {

// get index of the first true in bool array
template <size_t N>
inline constexpr size_t get_index_bool_array(std::array<bool, N> arr, size_t idx = 0) {
    if (idx == N) {
        return N;
    }
    if (arr[idx]) {
        return idx;
    }
    return get_index_bool_array(arr, idx + 1);
}

template <typename U, typename First, typename... Rest>
constexpr size_t get_type_index() {
    if constexpr (std::is_same_v<U, First>) {
        return 0;
    }
    else {
        return 1 + get_type_index<U, Rest...>();
    }
}

// get index of class in classes, with exact match
template <class T, class... Ts>
constexpr size_t get_cls_pos_v = get_index_bool_array(std::array{std::is_same_v<T, Ts>...});
// get index of first (sub)-class in classes
// the first match with sub class
template <class T, class... Ts>
constexpr size_t get_sub_cls_pos_v = get_index_bool_array(std::array{std::is_base_of_v<T, Ts>...});

// define what types are retrievable using sequence number
template <class... T>
struct RetrievableTypes;

// container default declaration
template <class... T>
class Container;
// real definition with retrievable types
template <class... GettableTypes, class... StorageableTypes>
class Container<RetrievableTypes<GettableTypes...>, StorageableTypes...> {
   public:
    static constexpr size_t num_storageable = sizeof...(StorageableTypes);
    static constexpr size_t num_gettable = sizeof...(GettableTypes);

    // default constructor, operator

    // reserve space
    template <class Storageable>
    void reserve(size_t size) {
        std::vector<Storageable>& vec = std::get<std::vector<Storageable>>(vectors_);
        vec.reserve(size);
    }

    // emplace component
    template <class Storageable, class... Args>
    void emplace(ID id, Args&&... args) {
        // template<class... Args> Args&&... args perfect forwarding
        assert(!construction_complete_);
        // throw if id already exists
        if (map_.contains(id)) {
            throw ConflictID{id};
        }
        // find group and position
        Idx const group = static_cast<Idx>(get_cls_pos_v<Storageable, StorageableTypes...>);
        std::vector<Storageable>& vec = std::get<std::vector<Storageable>>(vectors_);
        Idx const pos = static_cast<Idx>(vec.size());
        // create object
        vec.emplace_back(std::forward<Args>(args)...);
        // insert idx to map
        map_[id] = Idx2D{group, pos};
    }

    // get item based on Idx2D
    template <class Gettable>
    Gettable& get_item(Idx2D idx_2d) {
        constexpr std::array<GetItemFuncPtr<Gettable>, num_storageable> func_arr{
            select_get_item_func_ptr<Gettable, StorageableTypes>::ptr...};
        // selected group should be de derived class of Gettable
        assert(is_base<Gettable>[idx_2d.group]);
        return (this->*(func_arr[idx_2d.group]))(idx_2d.pos);
    }
    template <class Gettable>
    Gettable const& get_item(Idx2D idx_2d) const {
        constexpr std::array<GetItemFuncPtrConst<Gettable>, num_storageable> func_arr{
            select_get_item_func_ptr<Gettable, StorageableTypes>::ptr_const...};
        // selected group should be de derived class of Gettable
        assert(is_base<Gettable>[idx_2d.group]);
        return (this->*(func_arr[idx_2d.group]))(idx_2d.pos);
    }
    // get idx by id
    template <class Gettable = void>
    Idx2D get_idx_by_id(ID id) const {
        auto const found = map_.find(id);
        if (found == map_.end()) {
            throw IDNotFound{id};
        }
        if constexpr (!std::is_void_v<Gettable>) {
            if (!is_base<Gettable>[found->second.group]) {
                throw IDWrongType{id};
            }
        }
        return found->second;
    }
    // get item based on ID
    template <class Gettable>
    Gettable& get_item(ID id) {
        Idx2D const idx = get_idx_by_id<Gettable>(id);
        return get_item<Gettable>(idx);
    }
    template <class Gettable>
    Gettable const& get_item(ID id) const {
        Idx2D const idx = get_idx_by_id<Gettable>(id);
        return get_item<Gettable>(idx);
    }
    // get item based on sequence
    template <class Gettable>
    Gettable& get_item_by_seq(Idx seq) {
        assert(construction_complete_);
        return get_item<Gettable>(get_idx_2d_by_seq<Gettable>(seq));
    }
    template <class Gettable>
    Gettable const& get_item_by_seq(Idx seq) const {
        assert(construction_complete_);
        return get_item<Gettable>(get_idx_2d_by_seq<Gettable>(seq));
    }

    // get size
    template <class Gettable>
    Idx size() const {
        assert(construction_complete_);
        return size_[get_cls_pos_v<Gettable, GettableTypes...>];
    }

    // get sequence idx based on id
    template <class Gettable>
    Idx get_seq(ID id) const {
        assert(construction_complete_);
        std::array<Idx, num_storageable + 1> const& cum_size = cum_size_[get_cls_pos_v<Gettable, GettableTypes...>];
        auto const found = map_.find(id);
        assert(found != map_.end());
        return cum_size[found->second.group] + found->second.pos;
    }

    // get idx_2d based on sequence
    template <class Gettable>
    Idx2D get_idx_2d_by_seq(Idx seq) const {
        assert(construction_complete_);
        assert(seq >= 0);
        std::array<Idx, num_storageable + 1> const& cum_size = cum_size_[get_cls_pos_v<Gettable, GettableTypes...>];
        auto const found = std::upper_bound(cum_size.begin(), cum_size.end(), seq);
        assert(found != cum_size.end());
        Idx2D res;
        res.group = static_cast<Idx>(std::distance(cum_size.cbegin(), found) - 1);
        res.pos = seq - cum_size[res.group];
        return res;
    }

    // get start idx based on two classes
    // the GettableBaseType specifies the iterator range of all components whish is subclass of GettableBaseType
    // the StorageableSubType specifies a subset of iterator range of GettableBaseType
    // the function returns the start index of the first StorageableSubType (or its subclass)
    //      in the iterator range of U
    template <class GettableBaseType, class StorageableSubType>
    Idx get_start_idx() const {
        std::array<Idx, num_storageable + 1> const& cum_size =
            cum_size_[get_cls_pos_v<GettableBaseType, GettableTypes...>];
        return cum_size[get_sub_cls_pos_v<StorageableSubType, StorageableTypes...>];
    }

    template <class Storageable>
    Idx get_type_idx() const {
        return static_cast<Idx>(get_type_index<Storageable, StorageableTypes...>());
    }

    void set_construction_complete() {
#ifndef NDEBUG
        // set construction complete for debug assertions
        construction_complete_ = true;
#endif  // !NDEBUG
        size_ = {size_per_type<GettableTypes>()...};
        cum_size_ = {accumulate_size_per_vector<GettableTypes>()...};
    };

    // cache a Storagable item with index pos to restore to when restore_values() is called
    template <class Storageable>
    void cache_item(Idx pos) {
        const auto& value = get_raw<Storageable, Storageable>(pos);
        auto& cached_vec = std::get<std::vector<std::pair<Idx, Storageable>>>(cached_reset_values_);

        cached_vec.emplace_back(pos, value);
    }

    void restore_values() {
        (restore_values_impl<StorageableTypes>(), ...);
    }

   private:
    std::tuple<std::vector<StorageableTypes>...> vectors_;
    std::unordered_map<ID, Idx2D> map_;
    std::array<Idx, num_gettable> size_;
    std::array<std::array<Idx, num_storageable + 1>, num_gettable> cum_size_;

    std::tuple<std::vector<std::pair<Idx, StorageableTypes>>...> cached_reset_values_;  // indices + reset values

#ifndef NDEBUG
    // set construction_complete is used for debug assertions only
    bool construction_complete_{false};
#endif  // !NDEBUG

    // get item per type
    template <class GettableBaseType, class StorageableSubType>
    requires std::derived_from<StorageableSubType, GettableBaseType> GettableBaseType& get_raw(Idx pos) {
        return std::get<std::vector<StorageableSubType>>(vectors_)[pos];
    }
    template <class GettableBaseType, class StorageableSubType>
    requires std::derived_from<StorageableSubType, GettableBaseType> GettableBaseType const& get_raw(Idx pos) const {
        return std::get<std::vector<StorageableSubType>>(vectors_)[pos];
    }

    // templates to select function pointer
    template <class Storageable>
    using GetItemFuncPtr = Storageable& (Container::*)(Idx pos);
    template <class Storageable>
    using GetItemFuncPtrConst = Storageable const& (Container::*)(Idx pos) const;
    template <class GettableBaseType, class StorageableSubType, class = void>
    struct select_get_item_func_ptr {
        static constexpr GetItemFuncPtr<GettableBaseType> ptr = nullptr;
        static constexpr GetItemFuncPtrConst<GettableBaseType> ptr_const = nullptr;
    };
    template <class GettableBaseType, class StorageableSubType>
    requires std::derived_from<StorageableSubType, GettableBaseType>
    struct select_get_item_func_ptr<GettableBaseType, StorageableSubType> {
        static constexpr GetItemFuncPtr<GettableBaseType> ptr =
            &Container::get_raw<GettableBaseType, StorageableSubType>;
        static constexpr GetItemFuncPtrConst<GettableBaseType> ptr_const =
            &Container::get_raw<GettableBaseType, StorageableSubType>;
    };

    // array of base judge
    template <class Gettable>
    static constexpr std::array<bool, num_storageable> is_base{std::is_base_of_v<Gettable, StorageableTypes>...};
    // array of relevant vector size, for a non-derived class, the size is zero
    template <class Gettable>
    std::array<Idx, num_storageable> size_per_vector() const {
        assert(construction_complete_);
        return std::array<Idx, num_storageable>{
            std::is_base_of_v<Gettable, StorageableTypes>
                ? static_cast<Idx>(std::get<std::vector<StorageableTypes>>(vectors_).size())
                : 0 ...};
    }
    // total size of a type
    template <class Gettable>
    Idx size_per_type() const {
        assert(construction_complete_);
        std::array<Idx, num_storageable> const size_vec = size_per_vector<Gettable>();
        return std::reduce(size_vec.begin(), size_vec.end(), Idx{});
    }
    template <class Gettable>
    std::array<Idx, num_storageable + 1> accumulate_size_per_vector() const {
        assert(construction_complete_);
        std::array<Idx, num_storageable> const size_vec = size_per_vector<Gettable>();
        std::array<Idx, num_storageable + 1> res{};
        std::inclusive_scan(size_vec.begin(), size_vec.end(), res.begin() + 1);
        return res;
    }

    template <class Storageable>
    void restore_values_impl() {
        auto& cached_vec = std::get<std::vector<std::pair<Idx, Storageable>>>(cached_reset_values_);
        for (auto it = cached_vec.crbegin(); it != cached_vec.crend(); ++it) {
            auto const& cache = *it;
            get_raw<Storageable, Storageable>(cache.first) = cache.second;
        }
        cached_vec.clear();
    }

   private:
    // define iterator
    template <class Gettable>
    class Iterator : public boost::iterator_facade<Iterator<Gettable>, Gettable, boost::random_access_traversal_tag,
                                                   Gettable&, Idx> {
       public:
        static constexpr bool is_const = std::is_const_v<Gettable>;
        using base_type = std::remove_cv_t<Gettable>;
        using container_type = std::conditional_t<is_const, Container const, Container>;
        // constructor including default
        explicit Iterator(container_type* container_ptr = nullptr, Idx idx = 0)
            : container_ptr_{container_ptr}, idx_{idx} {
        }
        // conversion to const iterator
        template <class ConstGettable = Gettable>
        requires(!is_const) explicit operator Iterator<ConstGettable const>() const {
            return Iterator<ConstGettable const>{container_ptr_, idx_};
        }

       private:
        friend class boost::iterator_core_access;

        Gettable& dereference() const {
            return container_ptr_->template get_item_by_seq<base_type>(idx_);
        }
        bool equal(Iterator const& other) const {
            assert(container_ptr_ == other.container_ptr_);
            return idx_ == other.idx_;
        }
        void increment() {
            ++idx_;
        }
        void decrement() {
            --idx_;
        }
        void advance(Idx n) {
            idx_ += n;
        }
        Idx distance_to(Iterator const& other) const {
            assert(container_ptr_ == other.container_ptr_);
            return other.idx_ - idx_;
        }
        // store container pointer
        // and idx
        container_type* container_ptr_;
        Idx idx_;
    };

    // define proxy
    template <class Gettable>
    class Proxy {
       private:
        static constexpr bool is_const = std::is_const_v<Gettable>;
        using base_type = std::remove_cv_t<Gettable>;
        using container_type = std::conditional_t<is_const, Container const, Container>;

       public:
        explicit Proxy(container_type& container)
            : begin_{&container, 0}, end_{&container, container.template size<base_type>()} {
        }
        Iterator<Gettable> begin() {
            return begin_;
        }
        Iterator<Gettable> end() {
            return end_;
        }

       private:
        Iterator<Gettable> const begin_;
        Iterator<Gettable> const end_;
    };

   public:
    template <class Gettable>
    Proxy<Gettable> iter() {
        return Proxy<Gettable>{*this};
    }
    template <class Gettable>
    Proxy<Gettable const> iter() const {
        return Proxy<Gettable const>{*this};
    }
    template <class Gettable>
    Proxy<Gettable const> citer() const {
        return iter<Gettable>();
    }
};

// type traits to instantiate container
template <class... T>
struct ExtraRetrievableTypes;
// with no extra types, default all vector value types
template <class... T>
struct container_trait {
    using type = Container<RetrievableTypes<T...>, T...>;
};
// if extra types are provided, also add to the retrievable types
template <class... TR, class... T>
struct container_trait<ExtraRetrievableTypes<TR...>, T...> {
    using type = Container<RetrievableTypes<T..., TR...>, T...>;
};

}  // namespace container_impl

template <class... T>
using ExtraRetrievableTypes = container_impl::ExtraRetrievableTypes<T...>;

template <class... T>
using Container = typename container_impl::container_trait<T...>::type;

}  // namespace power_grid_model

#endif
