// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CPP_DATASET_HPP
#define POWER_GRID_MODEL_CPP_DATASET_HPP

#include "power_grid_model_c/dataset.h"

#include "basics.hpp"
#include "handle.hpp"
#include "serialization.hpp"

#include <concepts>

namespace power_grid_model_cpp {
class Info {
  private:     
    template<typename T>
    concept InfoLike = std::same_as<T, Info> || std::same_as<T, Dataset> || std::same_as<T, Component>;

  public:
    class Dataset {
      public:
        template <InfoLike T>
        static std::string const name(T const& info) {
            auto const name = std::string(PGM_dataset_info_name(info.handle_.get(), info.info_.get()));
            info.handle_.check_error();
            return name;
        }
        std::string const name() const { return name(*this); }

        template <InfoLike T>
        static Idx is_batch(T const& info) {
            auto const is_batch = PGM_dataset_info_is_batch(info.handle_.get(), info.info_.get());
            info.handle_.check_error();
            return is_batch;
        }
        Idx is_batch() const { return is_batch(*this); }

        template <InfoLike T>
        static Idx batch_size(T const& info) {
            auto const batch_size = PGM_dataset_info_batch_size(info.handle_.get(), info.info_.get());
            info.handle_.check_error();
            return batch_size;
        }
        Idx batch_size() const { return batch_size(*this); }

        template <InfoLike T>
        static Idx n_components(T const& info) {
            auto const n_components = PGM_dataset_info_n_components(info.handle_.get(), info.info_.get());
            info.handle_.check_error();
            return n_components;
        }
        Idx n_components() const { return n_components(*this); }

      private:
        friend class Info;
        Dataset(Handle& handle, std::unique_ptr<PGM_DatasetInfo const>& info) : handle_{handle}, info_{info} {}
        Handle& handle_;
        std::unique_ptr<PGM_DatasetInfo const>& info_;
    };

    class Component {
      public:
        template <InfoLike T>
        static std::string const name(T const& info, Idx component_idx) {
            auto const component_name = std::string(PGM_dataset_info_component_name(info.handle_.get(), info.info_.get(), component_idx));
            info.handle_.check_error();
            return component_name;
        }
        std::string const name(Idx component_idx) const {
            return name(*this, component_idx);
        }

        template <InfoLike T>
        static Idx elements_per_scenario(T const& info, Idx component_idx) {
            auto const elements_per_scenario =
                PGM_dataset_info_elements_per_scenario(info.handle_.get(), info.info_.get(), component_idx);
            info.handle_.check_error();
            return elements_per_scenario;
        }
        Idx elements_per_scenario(Idx component_idx) const {
            return elements_per_scenario(*this, component_idx);
        }

        template <InfoLike T>
        static Idx total_elements(T const& info, Idx component_idx) {
            auto const total_elements = PGM_dataset_info_total_elements(info.handle_.get(), info.info_.get(), component_idx);
            info.handle_.check_error();
            return total_elements;
        }
        Idx total_elements(Idx component_idx) const {
            return total_elements(*this, component_idx);
        }

      private:
        friend class Info;
        Component(Handle& handle, std::unique_ptr<PGM_DatasetInfo const>& info) : handle_{handle}, info_{info} {}
        Handle& handle_;
        std::unique_ptr<PGM_DatasetInfo const>& info_;
    };

    Dataset dataset;
    Component component;

  private:
    friend class Dataset;
    friend class Component;
    friend class DatasetConst;
    friend class DatasetWritable;
    friend class DatasetMutable;
        
    Info(PGM_DatasetInfo const* info) : info_{info}, dataset{handle_, info_}, component{handle_, info_} {}

    Handle handle_{};
    std::unique_ptr<PGM_DatasetInfo const> info_;
};

class DatasetConst {
  public:
    DatasetConst(std::string const& dataset, Idx is_batch, Idx batch_size)
        : dataset_{PGM_create_dataset_const(handle_.get(), dataset.c_str(), is_batch, batch_size)} {}
    DatasetConst(DatasetWritable const& writable_dataset)
        : dataset_{PGM_create_dataset_const_from_writable(handle_.get(), writable_dataset.dataset_.get())} {}
    DatasetConst(DatasetMutable const& mutable_dataset)
        : dataset_{PGM_create_dataset_const_from_mutable(handle_.get(), mutable_dataset.dataset_.get())} {}

    static void add_buffer(DatasetConst& dataset, std::string const& component, Idx elements_per_scenario,
                           Idx total_elements, Idx const* indptr, RawDataConstPtr data) {
        PGM_dataset_const_add_buffer(dataset.handle_.get(), dataset.dataset_.get(), component.c_str(), elements_per_scenario,
                                     total_elements, indptr, data);
        dataset.handle_.check_error();
    }
    void add_buffer(std::string const& component, Idx elements_per_scenario, Idx total_elements, Idx const* indptr,
                    RawDataConstPtr data) {
        return add_buffer(*this, component, elements_per_scenario, total_elements, indptr, data);
    }

    static Info const get_info(DatasetConst const& dataset) {
        auto const info = PGM_dataset_const_get_info(dataset.handle_.get(), dataset.dataset_.get());
        dataset.handle_.check_error();
        return Info(info);
    }
    Info const get_info() const { return get_info(*this); }

  private:
    friend class DatasetWritable;
    friend class DatasetMutable;
    Handle handle_{};
    detail::UniquePtr<PGM_ConstDataset, PGM_destroy_dataset_const> dataset_;
};

class DatasetWritable {
  public:
    static Info const get_info(DatasetWritable const& dataset) {
        auto const info = PGM_dataset_writable_get_info(dataset.handle_.get(), dataset.dataset_.get());
        dataset.handle_.check_error();
        return Info(info);
    }
    Info const get_info() const { return get_info(*this); }

    static void set_buffer(DatasetWritable const& dataset, std::string const& component, Idx* indptr, RawDataPtr data) {
        PGM_dataset_writable_set_buffer(dataset.handle_.get(), dataset.dataset_.get(), component.c_str(), indptr, data);
        dataset.handle_.check_error();
    }
    void set_buffer(std::string const& component, Idx* indptr, RawDataPtr data) {
        return set_buffer(*this, component.c_str(), indptr, data);
    }

  private:
    friend class Deserializer;
    friend class DatasetConst;
    DatasetWritable(PGM_WritableDataset* dataset) : dataset_{dataset} {}
    Handle handle_{};
    std::unique_ptr<PGM_WritableDataset> dataset_;
};

class DatasetMutable {
    DatasetMutable(std::string const& dataset, Idx is_batch, Idx batch_size)
        : dataset_{PGM_create_dataset_mutable(handle_.get(), dataset.c_str(), is_batch, batch_size)} {}

    static void add_buffer(DatasetMutable& dataset, std::string const& component, Idx elements_per_scenario,
                           Idx total_elements, Idx const* indptr, RawDataPtr data) {
        PGM_dataset_mutable_add_buffer(dataset.handle_.get(), dataset.dataset_.get(), component.c_str(), elements_per_scenario,
                                       total_elements, indptr, data);
        dataset.handle_.check_error();
    }
    void add_buffer(std::string const& component, Idx elements_per_scenario, Idx total_elements, Idx const* indptr,
                    RawDataPtr data) {
        return add_buffer(*this, component.c_str(), elements_per_scenario, total_elements, indptr, data);
    }

    static Info const get_info(DatasetMutable const& dataset) {
        auto const info = PGM_dataset_mutable_get_info(dataset.handle_.get(), dataset.dataset_.get());
        dataset.handle_.check_error();
        return Info(info);
    }
    Info const get_info() const { return get_info(*this); }

  private:
    friend class DatasetConst;
    friend class DatasetInfo;
    Handle handle_{};
    detail::UniquePtr<PGM_MutableDataset, PGM_destroy_dataset_mutable> dataset_;
};
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_DATASET_HPP
