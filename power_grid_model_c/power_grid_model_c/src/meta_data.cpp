// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_DLL_EXPORTS
#include "forward_declarations.hpp"

#include "power_grid_model_c/meta_data.h"

#include "get_meta_data.hpp"
#include "handle.hpp"
#include "input_sanitization.hpp"

#include <power_grid_model/auxiliary/meta_data_gen.hpp>
#include <power_grid_model/auxiliary/static_asserts/input.hpp>
#include <power_grid_model/auxiliary/static_asserts/output.hpp>
#include <power_grid_model/auxiliary/static_asserts/update.hpp>

namespace {
using namespace power_grid_model;

using power_grid_model_c::call_with_catch;
using power_grid_model_c::safe_ptr_get;
using power_grid_model_c::safe_str_view;
using power_grid_model_c::to_c_bool;
using power_grid_model_c::to_c_enum;

// assert index type
static_assert(std::is_same_v<PGM_Idx, Idx>);
static_assert(std::is_same_v<PGM_ID, ID>);

struct RangedExceptionHandler : public power_grid_model_c::DefaultExceptionHandler {
    void operator()(PGM_Handle& handle) const noexcept {
        using namespace std::string_literals;

        std::exception_ptr const ex_ptr = std::current_exception();
        try {
            std::rethrow_exception(ex_ptr);
        } catch (std::out_of_range const& ex) {
            handle_regular_error(handle, ex, PGM_regular_error, "\n You supplied wrong name and/or index!\n");
        } catch (std::exception const& ex) {
            handle_regular_error(handle, ex, PGM_regular_error);
        } catch (...) { // NOSONAR(S2738)
            handle_unkown_error(handle);
        }
    }
};

constexpr RangedExceptionHandler ranged_exception_handler{};
} // namespace

// retrieve meta data
power_grid_model::meta_data::MetaData const& get_meta_data() {
    return power_grid_model::meta_data::meta_data_gen::meta_data;
}

// dataset
PGM_Idx PGM_meta_n_datasets(PGM_Handle* /* handle */) { return get_meta_data().n_datasets(); }
PGM_MetaDataset const* PGM_meta_get_dataset_by_idx(PGM_Handle* handle, PGM_Idx idx) {
    return call_with_catch(
        handle,
        [idx] {
            if (idx < 0 || idx >= get_meta_data().n_datasets()) {
                throw std::out_of_range{"Index out of range!\n"};
            }
            return &get_meta_data().datasets[idx];
        },
        ranged_exception_handler);
}
PGM_MetaDataset const* PGM_meta_get_dataset_by_name(PGM_Handle* handle, char const* dataset) {
    return call_with_catch(
        handle, [dataset] { return &get_meta_data().get_dataset(safe_str_view(dataset)); }, ranged_exception_handler);
}
char const* PGM_meta_dataset_name(PGM_Handle* handle, PGM_MetaDataset const* dataset) {
    return call_with_catch(handle, [dataset] { return safe_ptr_get(dataset).name; });
}
// component
PGM_Idx PGM_meta_n_components(PGM_Handle* handle, PGM_MetaDataset const* dataset) {
    return call_with_catch(handle, [dataset] { return safe_ptr_get(dataset).n_components(); });
}
PGM_MetaComponent const* PGM_meta_get_component_by_idx(PGM_Handle* handle, PGM_MetaDataset const* dataset,
                                                       PGM_Idx idx) {
    return call_with_catch(
        handle,
        [idx, dataset] {
            auto const& safe_dataset = safe_ptr_get(dataset);
            if (idx < 0 || idx >= safe_dataset.n_components()) {
                throw std::out_of_range{"Index out of range!\n"};
            }
            return &safe_dataset.components[idx];
        },
        ranged_exception_handler);
}
PGM_MetaComponent const* PGM_meta_get_component_by_name(PGM_Handle* handle, char const* dataset,
                                                        char const* component) {
    return call_with_catch(
        handle,
        [component, dataset] {
            return &get_meta_data().get_dataset(safe_str_view(dataset)).get_component(safe_str_view(component));
        },
        ranged_exception_handler);
}
char const* PGM_meta_component_name(PGM_Handle* handle, PGM_MetaComponent const* component) {
    return call_with_catch(handle, [component] { return safe_ptr_get(component).name; });
}
size_t PGM_meta_component_size(PGM_Handle* handle, PGM_MetaComponent const* component) {
    return call_with_catch(handle, [component] { return safe_ptr_get(component).size; });
}
size_t PGM_meta_component_alignment(PGM_Handle* handle, PGM_MetaComponent const* component) {
    return call_with_catch(handle, [component] { return safe_ptr_get(component).alignment; });
}
// attributes
PGM_Idx PGM_meta_n_attributes(PGM_Handle* handle, PGM_MetaComponent const* component) {
    return call_with_catch(handle, [component] { return safe_ptr_get(component).n_attributes(); });
}
PGM_MetaAttribute const* PGM_meta_get_attribute_by_idx(PGM_Handle* handle, PGM_MetaComponent const* component,
                                                       PGM_Idx idx) {
    return call_with_catch(
        handle,
        [idx, component] {
            auto const& safe_component = safe_ptr_get(component);
            if (idx < 0 || idx >= safe_component.n_attributes()) {
                throw std::out_of_range{"Index out of range!\n"};
            }
            return &safe_component.attributes[idx];
        },
        ranged_exception_handler);
}
PGM_MetaAttribute const* PGM_meta_get_attribute_by_name(PGM_Handle* handle, char const* dataset, char const* component,
                                                        char const* attribute) {
    return call_with_catch(
        handle,
        [component, dataset, attribute] {
            return &get_meta_data()
                        .get_dataset(safe_str_view(dataset))
                        .get_component(safe_str_view(component))
                        .get_attribute(safe_str_view(attribute));
        },
        ranged_exception_handler);
}
char const* PGM_meta_attribute_name(PGM_Handle* handle, PGM_MetaAttribute const* attribute) {
    return call_with_catch(handle, [attribute] { return safe_ptr_get(attribute).name; });
}
PGM_Idx PGM_meta_attribute_ctype(PGM_Handle* handle, PGM_MetaAttribute const* attribute) {
    return call_with_catch(handle, [attribute] { return to_c_enum(safe_ptr_get(attribute).ctype); });
}
size_t PGM_meta_attribute_offset(PGM_Handle* handle, PGM_MetaAttribute const* attribute) {
    return call_with_catch(handle, [attribute] { return safe_ptr_get(attribute).offset; });
}
int PGM_is_little_endian(PGM_Handle* /* handle */) { return to_c_bool<int>(meta_data::is_little_endian()); }
