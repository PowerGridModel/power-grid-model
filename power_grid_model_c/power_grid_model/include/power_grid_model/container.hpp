// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

// container for multiple components

#include "common/common.hpp"
#include "common/exception.hpp"
#include "common/iterator_facade.hpp"

#include <boost/range.hpp>

#include <array>
#include <functional>
#include <memory>
#include <numeric>
#include <span>
#include <unordered_map>

namespace power_grid_model {

namespace container_impl {

// get index of the first true in bool array
template <size_t N> constexpr size_t get_index_bool_array(std::array<bool, N> arr, size_t idx = 0) {
    if (idx == N) {
        return N;
    }
    if (arr[idx]) {
        return idx;
    }
    return get_index_bool_array(arr, idx + 1);
}

template <typename U, typename First, typename... Rest> constexpr size_t get_type_index() {
    if constexpr (std::is_same_v<U, First>) {
        return 0;
    } else {
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

template <class T, class... SupportedTs>
concept supported_type_c = is_in_list_c<T, SupportedTs...>;

// define what types are retrievable using sequence number
template <class... T> struct RetrievableTypes;

// container default declaration
template <class... T> class Container;
// real definition with retrievable types
template <class... GettableTypes, class... StorageableTypes>
class Container<RetrievableTypes<GettableTypes...>, StorageableTypes...> {
  public:
    using gettable_types = std::tuple<GettableTypes...>;

    static constexpr size_t num_storageable = sizeof...(StorageableTypes);
    static constexpr size_t num_gettable = sizeof...(GettableTypes);

    // default constructor, operator

    template <typename T> static constexpr bool is_storageable_v = supported_type_c<T, StorageableTypes...>;
    template <typename T> static constexpr bool is_gettable_v = supported_type_c<T, GettableTypes...>;

    // reserve space
    template <supported_type_c<StorageableTypes...> Storageable> void reserve(size_t size) {
        auto& vec = std::get<std::vector<Storageable>>(vectors_);
        vec.reserve(size);
    }

    // emplace component
    template <supported_type_c<StorageableTypes...> Storageable, class... Args> void emplace(ID id, Args&&... args) {
        // template<class... Args> Args&&... args perfect forwarding
        assert(!construction_complete_);
        // throw if id already exists
        if (map_.contains(id)) {
            throw ConflictID{id};
        }
        // find group and position
        auto const group = static_cast<Idx>(get_cls_pos_v<Storageable, StorageableTypes...>);
        auto& vec = std::get<std::vector<Storageable>>(vectors_);
        auto const pos = static_cast<Idx>(vec.size());
        // create object
        vec.emplace_back(std::forward<Args>(args)...);
        // insert idx to map
        map_[id] = Idx2D{.group = group, .pos = pos};
    }

    // get item based on Idx2D
    template <supported_type_c<GettableTypes...> Gettable> Gettable& get_item(Idx2D idx_2d) {
        constexpr std::array<GetItemFuncPtr<Gettable>, num_storageable> func_arr{
            select_get_item_func_ptr<Gettable, StorageableTypes>::ptr...};
        // selected group should be de derived class of Gettable
        assert(is_base<Gettable>[idx_2d.group]);
        return (this->*(func_arr[idx_2d.group]))(idx_2d.pos);
    }
    template <supported_type_c<GettableTypes...> Gettable> Gettable const& get_item(Idx2D idx_2d) const {
        constexpr std::array<GetItemFuncPtrConst<Gettable>, num_storageable> func_arr{
            select_get_item_func_ptr<Gettable, StorageableTypes>::ptr_const...};
        // selected group should be de derived class of Gettable
        assert(is_base<Gettable>[idx_2d.group]);
        return (this->*(func_arr[idx_2d.group]))(idx_2d.pos);
    }

#ifndef NDEBUG
    // get id by idx, only for debugging purpose
    ID get_id_by_idx(Idx2D idx_2d) const {
        if (auto it = std::ranges::find(map_, idx_2d, &std::pair<const ID, Idx2D>::second); it != map_.end()) {
            return it->first;
        }
        throw Idx2DNotFound{idx_2d};
    }
#endif // NDEBUG

    // get idx by id
    Idx2D get_idx_by_id(ID id) const {
        auto const found = map_.find(id);
        if (found == map_.end()) {
            throw IDNotFound{id};
        }
        return found->second;
    }
    template <supported_type_c<GettableTypes...> Gettable> Idx2D get_idx_by_id(ID id) const {
        auto const result = get_idx_by_id(id);
        if (!is_base<Gettable>[result.group]) {
            throw IDWrongType{id};
        }
        return result;
    }
    template <supported_type_c<StorageableTypes...> Storageable> constexpr Idx get_group_idx() const {
        return static_cast<Idx>(get_cls_pos_v<Storageable, StorageableTypes...>);
    }

    // get item based on ID
    template <supported_type_c<GettableTypes...> Gettable> Gettable& get_item(ID id) {
        Idx2D const idx = get_idx_by_id<Gettable>(id);
        return get_item<Gettable>(idx);
    }
    template <supported_type_c<GettableTypes...> Gettable> Gettable const& get_item(ID id) const {
        Idx2D const idx = get_idx_by_id<Gettable>(id);
        return get_item<Gettable>(idx);
    }
    // get item based on sequence
    template <supported_type_c<GettableTypes...> Gettable> Gettable& get_item_by_seq(Idx seq) {
        assert(construction_complete_);
        return get_item<Gettable>(get_idx_2d_by_seq<Gettable>(seq));
    }
    template <supported_type_c<GettableTypes...> Gettable> Gettable const& get_item_by_seq(Idx seq) const {
        assert(construction_complete_);
        return get_item<Gettable>(get_idx_2d_by_seq<Gettable>(seq));
    }

    // get size
    template <supported_type_c<GettableTypes...> Gettable> Idx size() const {
        assert(construction_complete_);
        return size_[get_cls_pos_v<Gettable, GettableTypes...>];
    }

    // get sequence idx based on idx_2d
    // E.g. when you know the idx_2d of the derived class but want to know the index of the base class getter
    template <supported_type_c<GettableTypes...> Gettable> Idx get_seq(Idx2D idx_2d) const {
        assert(construction_complete_);
        std::array<Idx, num_storageable + 1> const& cum_size = cum_size_[get_cls_pos_v<Gettable, GettableTypes...>];
        return cum_size[idx_2d.group] + idx_2d.pos;
    }

    // get sequence idx based on id
    template <supported_type_c<GettableTypes...> Gettable> Idx get_seq(ID id) const {
        assert(construction_complete_);
        auto const found = map_.find(id);
        assert(found != map_.end());
        return get_seq<Gettable>(found->second);
    }

    // get idx_2d based on sequence
    template <supported_type_c<GettableTypes...> Gettable> Idx2D get_idx_2d_by_seq(Idx seq) const {
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
    template <supported_type_c<GettableTypes...> GettableBaseType, class StorageableSubType> Idx get_start_idx() const {
        std::array<Idx, num_storageable + 1> const& cum_size =
            cum_size_[get_cls_pos_v<GettableBaseType, GettableTypes...>];
        return cum_size[get_sub_cls_pos_v<StorageableSubType, StorageableTypes...>];
    }

    template <supported_type_c<StorageableTypes...> Storageable> static constexpr Idx get_type_idx() {
        return static_cast<Idx>(get_type_index<Storageable, StorageableTypes...>());
    }

    void set_construction_complete() {
#ifndef NDEBUG
        // set construction complete for debug assertions
        construction_complete_ = true;
#endif // !NDEBUG
        size_ = {size_per_type<GettableTypes>()...};
        cum_size_ = {accumulate_size_per_vector<GettableTypes>()...};
    };

  private:
    std::tuple<std::vector<StorageableTypes>...> vectors_;
    std::unordered_map<ID, Idx2D> map_;
    std::array<Idx, num_gettable> size_;
    std::array<std::array<Idx, num_storageable + 1>, num_gettable> cum_size_;

#ifndef NDEBUG
    // set construction_complete is used for debug assertions only
    bool construction_complete_{false};
#endif // !NDEBUG

    // get item per type
    template <supported_type_c<GettableTypes...> GettableBaseType, class StorageableSubType>
        requires std::derived_from<StorageableSubType, GettableBaseType>
    GettableBaseType& get_raw(Idx pos) {
        return std::get<std::vector<StorageableSubType>>(vectors_)[pos];
    }
    template <supported_type_c<GettableTypes...> GettableBaseType, class StorageableSubType>
        requires std::derived_from<StorageableSubType, GettableBaseType>
    GettableBaseType const& get_raw(Idx pos) const {
        return std::get<std::vector<StorageableSubType>>(vectors_)[pos];
    }

    // templates to select function pointer
    template <class Storageable> using GetItemFuncPtr = Storageable& (Container::*)(Idx pos);
    template <class Storageable> using GetItemFuncPtrConst = Storageable const& (Container::*)(Idx pos) const;
    template <supported_type_c<GettableTypes...> GettableBaseType, class StorageableSubType, class = void>
    struct select_get_item_func_ptr {
        static constexpr GetItemFuncPtr<GettableBaseType> ptr = nullptr;
        static constexpr GetItemFuncPtrConst<GettableBaseType> ptr_const = nullptr;
    };
    template <supported_type_c<GettableTypes...> GettableBaseType, class StorageableSubType>
        requires std::derived_from<StorageableSubType, GettableBaseType>
    struct select_get_item_func_ptr<GettableBaseType, StorageableSubType> {
        static constexpr GetItemFuncPtr<GettableBaseType> ptr =
            &Container::get_raw<GettableBaseType, StorageableSubType>;
        static constexpr GetItemFuncPtrConst<GettableBaseType> ptr_const =
            &Container::get_raw<GettableBaseType, StorageableSubType>;
    };

    // array of base judge
    template <supported_type_c<GettableTypes...> Gettable>
    static constexpr std::array<bool, num_storageable> is_base{std::is_base_of_v<Gettable, StorageableTypes>...};
    // array of relevant vector size, for a non-derived class, the size is zero
    template <supported_type_c<GettableTypes...> Gettable> std::array<Idx, num_storageable> size_per_vector() const {
        assert(construction_complete_);
        return std::array<Idx, num_storageable>{
            std::is_base_of_v<Gettable, StorageableTypes>
                ? static_cast<Idx>(std::get<std::vector<StorageableTypes>>(vectors_).size())
                : 0 ...};
    }
    // total size of a type
    template <supported_type_c<GettableTypes...> Gettable> Idx size_per_type() const {
        assert(construction_complete_);
        std::array<Idx, num_storageable> const size_vec = size_per_vector<Gettable>();
        return std::reduce(size_vec.begin(), size_vec.end(), Idx{});
    }
    template <supported_type_c<GettableTypes...> Gettable>
    std::array<Idx, num_storageable + 1> accumulate_size_per_vector() const {
        assert(construction_complete_);
        std::array<Idx, num_storageable> const size_vec = size_per_vector<Gettable>();
        std::array<Idx, num_storageable + 1> res{};
        std::inclusive_scan(size_vec.begin(), size_vec.end(), res.begin() + 1);
        return res;
    }

    // define iterator
    template <supported_type_c<GettableTypes...> Gettable>
    class Iterator : public IteratorFacade<Iterator<Gettable>, Gettable, Idx> {
      public:
        static constexpr bool is_const = std::is_const_v<Gettable>;
        using base_type = std::remove_cv_t<Gettable>;
        using container_type = std::conditional_t<is_const, Container const, Container>;
        // constructor including default
        explicit Iterator(container_type* container_ptr = nullptr, Idx idx = 0)
            : container_ptr_{container_ptr}, idx_{idx} {}
        // conversion to const iterator
        template <class ConstGettable = Gettable>
            requires(!is_const)
        explicit operator Iterator<ConstGettable const>() const {
            return Iterator<ConstGettable const>{container_ptr_, idx_};
        }

      private:
        friend class IteratorFacade<Iterator<Gettable>, Gettable, Idx>;

        constexpr Gettable const& dereference() const {
            return container_ptr_->template get_item_by_seq<base_type>(idx_);
        }
        constexpr Gettable& dereference() { return container_ptr_->template get_item_by_seq<base_type>(idx_); }
        constexpr auto three_way_compare(Iterator const& other) const {
            assert(container_ptr_ == other.container_ptr_);
            return idx_ <=> other.idx_;
        }
        constexpr void advance(Idx n) { idx_ += n; }
        constexpr Idx distance_to(Iterator const& other) const {
            assert(container_ptr_ == other.container_ptr_);
            return other.idx_ - idx_;
        }

        // store container pointer
        // and idx
        container_type* container_ptr_;
        Idx idx_;
    };

  public:
    template <supported_type_c<GettableTypes...> Gettable> auto iter() {
        return std::ranges::subrange{Iterator<Gettable>{this, 0},
                                     Iterator<Gettable>{this, this->template size<std::remove_cv_t<Gettable>>()}};
    }
    template <supported_type_c<GettableTypes...> Gettable> auto iter() const {
        return std::ranges::subrange{Iterator<Gettable const>{this, 0},
                                     Iterator<Gettable const>{this, this->template size<std::remove_cv_t<Gettable>>()}};
    }
    template <supported_type_c<GettableTypes...> Gettable> auto citer() const { return iter<Gettable>(); }
};

// type traits to instantiate container
template <class... T> struct ExtraRetrievableTypes;
// with no extra types, default all vector value types
template <class... T> struct container_trait {
    using type = Container<RetrievableTypes<T...>, T...>;
};
// if extra types are provided, also add to the retrievable types
template <class... TR, class... T> struct container_trait<ExtraRetrievableTypes<TR...>, T...> {
    using type = Container<RetrievableTypes<T..., TR...>, T...>;
};

} // namespace container_impl

template <class... T> using ExtraRetrievableTypes = container_impl::ExtraRetrievableTypes<T...>;

template <class... T> using Container = typename container_impl::container_trait<T...>::type;

} // namespace power_grid_model
