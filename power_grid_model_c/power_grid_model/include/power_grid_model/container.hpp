// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CONTAINER_HPP
#define POWER_GRID_MODEL_CONTAINER_HPP

// container for multiple components

#include <functional>
#include <memory>
#include <unordered_map>

#include "boost/iterator/iterator_facade.hpp"
#include "exception.hpp"
#include "power_grid_model.hpp"

namespace power_grid_model {

namespace container_impl {

// get index of the first true in bool array
template <size_t N>
inline constexpr size_t get_index_bool_array(std::array<bool, N> arr, size_t idx = 0) {
    if (idx == N)
        return N;
    if (arr[idx])
        return idx;
    return get_index_bool_array(arr, idx + 1);
}

template <typename U, typename First, typename... Rest>
constexpr size_t get_type_index() {
    if constexpr (std::is_same<U, First>::value)
        return 0;
    else
        return 1 + get_type_index<U, Rest...>();
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
template <class... TR, class... T>
class Container<RetrievableTypes<TR...>, T...> {
   public:
    static constexpr size_t N = sizeof...(T);
    static constexpr size_t NR = sizeof...(TR);

    // default constructor, operator

    // reserve space
    template <class U>
    void reserve(size_t size) {
        std::vector<U>& vec = std::get<std::vector<U>>(vectors_);
        vec.reserve(size);
    }

    // emplace component
    template <class U, class... Args>
    void emplace(ID id, Args&&... args) {
        // template<class... Args> Args&&... args perfect forwarding
        assert(!construction_complete_);
        // throw if id already exist
        if (map_.find(id) != map_.end()) {
            throw ConflictID{id};
        }
        // find group and position
        Idx const group = (Idx)get_cls_pos_v<U, T...>;
        std::vector<U>& vec = std::get<std::vector<U>>(vectors_);
        Idx const pos = (Idx)vec.size();
        // create object
        vec.emplace_back(std::forward<Args>(args)...);
        // insert idx to map
        map_[id] = Idx2D{group, pos};
    }

    // get item based on Idx2D
    template <class U>
    U& get_item(Idx2D idx_2d) {
        constexpr std::array<GetItemFuncPtr<U>, N> func_arr{select_get_item_func_ptr<U, T>::ptr...};
        // selected group should be de derived class of U
        assert(is_base<U>[idx_2d.group]);
        return (this->*(func_arr[idx_2d.group]))(idx_2d.pos);
    }
    template <class U>
    U const& get_item(Idx2D idx_2d) const {
        constexpr std::array<GetItemFuncPtrConst<U>, N> func_arr{select_get_item_func_ptr<U, T>::ptr_const...};
        // selected group should be de derived class of U
        assert(is_base<U>[idx_2d.group]);
        return (this->*(func_arr[idx_2d.group]))(idx_2d.pos);
    }
    // get idx by id
    template <class U = void>
    Idx2D get_idx_by_id(ID id) const {
        auto const found = map_.find(id);
        if (found == map_.end()) {
            throw IDNotFound{id};
        }
        if constexpr (!std::is_void_v<U>) {
            if (!is_base<U>[found->second.group]) {
                throw IDWrongType{id};
            }
        }
        return found->second;
    }
    // get item based on ID
    template <class U>
    U& get_item(ID id) {
        Idx2D const idx = get_idx_by_id<U>(id);
        return get_item<U>(idx);
    }
    template <class U>
    U const& get_item(ID id) const {
        Idx2D const idx = get_idx_by_id<U>(id);
        return get_item<U>(idx);
    }
    // get item based on sequence
    template <class U>
    U& get_item_by_seq(Idx seq) {
        assert(construction_complete_);
        return get_item<U>(get_idx_2d_by_seq<U>(seq));
    }
    template <class U>
    U const& get_item_by_seq(Idx seq) const {
        assert(construction_complete_);
        return get_item<U>(get_idx_2d_by_seq<U>(seq));
    }

    // get size
    template <class U>
    Idx size() const {
        assert(construction_complete_);
        return size_[get_cls_pos_v<U, TR...>];
    }

    // get sequence idx based on id
    template <class U>
    Idx get_seq(ID id) const {
        assert(construction_complete_);
        std::array<Idx, N + 1> const& cum_size = cum_size_[get_cls_pos_v<U, TR...>];
        auto const found = map_.find(id);
        assert(found != map_.end());
        return cum_size[found->second.group] + found->second.pos;
    }

    // get idx_2d based on sequence
    template <class U>
    Idx2D get_idx_2d_by_seq(Idx seq) const {
        assert(construction_complete_);
        assert(seq >= 0);
        std::array<Idx, N + 1> const& cum_size = cum_size_[get_cls_pos_v<U, TR...>];
        auto const found = std::upper_bound(cum_size.begin(), cum_size.end(), seq);
        assert(found != cum_size.end());
        Idx2D res;
        res.group = (Idx)std::distance(cum_size.cbegin(), found) - 1;
        res.pos = seq - cum_size[res.group];
        return res;
    }

    // get start idx based on two classes
    // the U specifies the iterator range of all components whish is subclass of U
    // the US specifies a sebset of iterator range of U
    // the function returns the start index of the first US (or its subclass)
    //      in the iterator range of U
    template <class U, class US>
    Idx get_start_idx() const {
        std::array<Idx, N + 1> const& cum_size = cum_size_[get_cls_pos_v<U, TR...>];
        return cum_size[get_sub_cls_pos_v<US, T...>];
    }

    template <class U>
    Idx get_type_idx() const {
        return (Idx)get_type_index<U, T...>();
    }

    void set_construction_complete() {
#ifndef NDEBUG
        // set construction complete for debug assertions
        construction_complete_ = true;
#endif  // !NDEBUG
        size_ = {size_per_type<TR>()...};
        cum_size_ = {accumulate_size_per_vector<TR>()...};
    };

   private:
    std::tuple<std::vector<T>...> vectors_;
    std::unordered_map<ID, Idx2D> map_;
    std::array<Idx, NR> size_;
    std::array<std::array<Idx, N + 1>, NR> cum_size_;

#ifndef NDEBUG
    // set construction_complete is used for debug assertions only
    bool construction_complete_{false};
#endif  // !NDEBUG

    // get item per type
    template <class U1, class U2>
    U1& get_item_type(Idx pos) {
        static_assert(std::is_base_of_v<U1, U2>);
        return std::get<std::vector<U2>>(vectors_)[pos];
    }
    template <class U1, class U2>
    U1 const& get_item_type(Idx pos) const {
        static_assert(std::is_base_of_v<U1, U2>);
        return std::get<std::vector<U2>>(vectors_)[pos];
    }

    // templates to select function pointer
    template <class U>
    using GetItemFuncPtr = U& (Container::*)(Idx pos);
    template <class U>
    using GetItemFuncPtrConst = U const& (Container::*)(Idx pos) const;
    template <class U1, class U2, class = void>
    struct select_get_item_func_ptr {
        static constexpr GetItemFuncPtr<U1> ptr = nullptr;
        static constexpr GetItemFuncPtrConst<U1> ptr_const = nullptr;
    };
    template <class U1, class U2>
    struct select_get_item_func_ptr<U1, U2, std::enable_if_t<std::is_base_of_v<U1, U2>>> {
        static constexpr GetItemFuncPtr<U1> ptr = &Container::get_item_type<U1, U2>;
        static constexpr GetItemFuncPtrConst<U1> ptr_const = &Container::get_item_type<U1, U2>;
    };

    // array of base judge
    template <class U>
    static constexpr std::array<bool, N> is_base{std::is_base_of_v<U, T>...};
    // array of relevant vector size, for a non-derived class, the size is zero
    template <class U>
    std::array<Idx, N> size_per_vector() const {
        assert(construction_complete_);
        return std::array<Idx, N>{std::is_base_of_v<U, T> ? (Idx)std::get<std::vector<T>>(vectors_).size() : 0 ...};
    }
    // total size of a type
    template <class U>
    Idx size_per_type() const {
        assert(construction_complete_);
        std::array<Idx, N> const size_vec = size_per_vector<U>();
        return std::reduce(size_vec.begin(), size_vec.end(), Idx{});
    }
    template <class U>
    std::array<Idx, N + 1> accumulate_size_per_vector() const {
        assert(construction_complete_);
        std::array<Idx, N> const size_vec = size_per_vector<U>();
        std::array<Idx, N + 1> res{};
        std::inclusive_scan(size_vec.begin(), size_vec.end(), res.begin() + 1);
        return res;
    }

   private:
    // define iterator
    template <class U>
    class Iterator : public boost::iterator_facade<Iterator<U>, U, boost::random_access_traversal_tag, U&, Idx> {
       public:
        static constexpr bool is_const = std::is_const_v<U>;
        using base_type = std::remove_cv_t<U>;
        using container_type = std::conditional_t<is_const, Container const, Container>;
        // constructor including default
        explicit Iterator(container_type* container_ptr = nullptr, Idx idx = 0)
            : container_ptr_{container_ptr}, idx_{idx} {
        }
        // conversion to const iterator
        template <class UX = U>
        operator std::enable_if_t<!is_const, Iterator<UX const>>() const {
            return Iterator<UX const>{container_ptr_, idx_};
        }

       private:
        friend class boost::iterator_core_access;

        U& dereference() const {
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
    template <class U>
    class Proxy {
       private:
        static constexpr bool is_const = std::is_const_v<U>;
        using base_type = std::remove_cv_t<U>;
        using container_type = std::conditional_t<is_const, Container const, Container>;

       public:
        Proxy(container_type& container)
            : begin_{&container, 0}, end_{&container, container.template size<base_type>()} {
        }
        Iterator<U> begin() {
            return begin_;
        }
        Iterator<U> end() {
            return end_;
        }

       private:
        Iterator<U> const begin_;
        Iterator<U> const end_;
    };

   public:
    template <class U>
    Proxy<U> iter() {
        return Proxy<U>{*this};
    }
    template <class U>
    Proxy<U const> iter() const {
        return Proxy<U const>{*this};
    }
    template <class U>
    Proxy<U const> citer() const {
        return iter<U>();
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
