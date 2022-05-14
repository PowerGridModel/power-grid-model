// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_MATH_SOLVER_Y_BUS_HPP
#define POWER_GRID_MODEL_MATH_SOLVER_Y_BUS_HPP

#include "../calculation_parameters.hpp"
#include "../power_grid_model.hpp"
#include "../sparse_mapping.hpp"
#include "../three_phase_tensor.hpp"

namespace power_grid_model {

// hide implementation in inside namespace
namespace math_model_impl {

using OffDiagIdxMap = std::array<Idx, 2>;  // map of ft and tf for branch

using MatrixPos = std::pair<Idx, Idx>;

struct YBusElementMap {
    MatrixPos pos;
    YBusElement element;
};

// append to element vector
inline void append_element_vector(std::vector<YBusElementMap>& vec, Idx bus1, Idx bus2, YBusElementType element_type,
                                  Idx idx) {
    // skip for -1
    if (bus1 == -1 || bus2 == -1) {
        return;
    }
    // add
    vec.push_back({{bus1, bus2}, {element_type, idx}});
}

// counting sort element
inline void counting_sort_element(std::vector<YBusElementMap>& vec, Idx n_bus) {
    // count vec
    std::vector<YBusElementMap> count_vec(vec.size());
    IdxVector counter(n_bus, 0);
    // sort column
    for (YBusElementMap const& element : vec) {
        ++counter[element.pos.second];
    }
    for (size_t i = 1, n = counter.size(); i != n; ++i) {
        counter[i] += counter[i - 1];
    }
    for (auto it_element = vec.crbegin(); it_element != vec.crend(); ++it_element) {
        count_vec[--counter[it_element->pos.second]] = *it_element;
    }
    // sort row
    std::fill(counter.begin(), counter.end(), 0);
    for (YBusElementMap const& element : count_vec) {
        ++counter[element.pos.first];
    }
    for (size_t i = 1, n = counter.size(); i != n; ++i) {
        counter[i] += counter[i - 1];
    }
    for (auto it_element = count_vec.crbegin(); it_element != count_vec.crend(); ++it_element) {
        vec[--counter[it_element->pos.first]] = *it_element;
    }
}

// y bus structure
struct YBusStructure {
    // csr structure
    IdxVector row_indptr;
    IdxVector col_indices;
    IdxVector row_indices;
    // element of ybus of all components
    std::vector<YBusElement> y_bus_element;
    std::vector<Idx> y_bus_entry_indptr;
    // sequence entry of bus data
    IdxVector bus_entry;
    // transpose entry of the sparse matrix
    // length of nnz Idx array.
    // for transpose_entry[i] indicates the position i-th element in transposed matrix in CSR form
    // for entry in the diagonal transpose_entry[i] = i
    IdxVector transpose_entry;

    // construct ybus structure
    YBusStructure(MathModelTopology const& topo) {
        Idx const n_bus = topo.n_bus();
        Idx const n_branch = (Idx)topo.branch_bus_idx.size();
        // allocate element vector
        std::vector<YBusElementMap> vec_map_element;
        Idx const total_number_entries = 4 * n_branch + n_bus;
        vec_map_element.reserve(total_number_entries);
        // add element
        // off diagonal element list
        std::vector<OffDiagIdxMap> off_diag_map(n_branch);
        // loop branch
        for (Idx branch = 0; branch != n_branch; ++branch) {
            // ff, ft, tf, tt for branch
            for (IntS i = 0; i != 4; ++i) {
                Idx const bus1 = topo.branch_bus_idx[branch][i / 2];  // 0, 0, 1, 1
                Idx const bus2 = topo.branch_bus_idx[branch][i % 2];  // 0, 1, 0, 1
                append_element_vector(vec_map_element, bus1, bus2, static_cast<YBusElementType>(i), branch);
            }
        }
        // loop shunt
        for (Idx bus = 0; bus != n_bus; ++bus) {
            for (Idx shunt = topo.shunt_bus_indptr[bus]; shunt != topo.shunt_bus_indptr[bus + 1]; ++shunt) {
                append_element_vector(vec_map_element, bus, bus, YBusElementType::shunt, shunt);
            }
        }
        // sort element
        counting_sort_element(vec_map_element, n_bus);
        // initialize nnz and row start
        Idx nnz_counter = 0;
        Idx row_start = 0;
        // allocate arrays
        row_indptr.resize(n_bus + 1);
        row_indptr[0] = 0;
        // allocate indices of entries
        y_bus_element.resize(vec_map_element.size());
        bus_entry.resize(n_bus);
        // start entry indptr as zero
        y_bus_entry_indptr.push_back(0);
        // copy all elements
        std::transform(vec_map_element.cbegin(), vec_map_element.cend(), y_bus_element.begin(), [](YBusElementMap m) {
            return m.element;
        });

        // iterate the whole element
        for (auto it_element = vec_map_element.cbegin(); it_element != vec_map_element.cend();
             // incremental happends in the inner loop, not here
        ) {
            MatrixPos const pos = it_element->pos;
            // row and col
            Idx const row = pos.first;
            Idx const col = pos.second;
            // assign col
            col_indices.push_back(col);
            row_indices.push_back(row);
            // iterate row if needed
            if (row > row_start) {
                row_indptr[++row_start] = nnz_counter;
            }
            // every row should have entries
            // so the row start (after increment once) should be the same as current row
            assert(row_start == row);
            // inner loop to assign entries of duplicated elements
            for (  // use it_element to start
                ;  // stop when reach end or new position
                it_element != vec_map_element.cend() && it_element->pos == pos; ++it_element) {
                // assign nnz as indices for bus diag entry, or record off diag entry
                // bus, diag entry
                if (row == col) {
                    bus_entry[row] = nnz_counter;
                }
                // branch off diag entry
                else {
                    off_diag_map[it_element->element.idx]
                                // minus 1 because ft is 1 and tf is 2, mapped to 0 and 1
                                [static_cast<Idx>(it_element->element.element_type) - 1] = nnz_counter;
                }
            }
            // all entries in the same position are looped, append indptr
            y_bus_entry_indptr.push_back((Idx)(it_element - vec_map_element.cbegin()));
            // iterate linear nnz
            ++nnz_counter;
        }
        // last entry for indptr
        row_indptr[++row_start] = nnz_counter;
        // for empty shunt and branch, add artificial one element
        if (topo.n_branch() == 0 && topo.n_shunt() == 0) {
            assert(n_bus == 1);
            nnz_counter = 1;
            row_indptr = {0, 1};
            col_indices = {0};
            row_indices = {0};
            bus_entry = {0};
            transpose_entry = {0};
            y_bus_entry_indptr = {0, 0};
        }
        // no empty row is allowed
        assert(row_start == n_bus);
        // size of y_bus_entry_indptr is nnz + 1
        assert((Idx)y_bus_entry_indptr.size() == nnz_counter + 1);
        // end of y_bus_entry_indptr is same as size of entry
        assert(y_bus_entry_indptr.back() == (Idx)y_bus_element.size());

        // construct transpose entry
        transpose_entry.resize(nnz_counter);
        // default transpose_entry[i] = i
        std::iota(transpose_entry.begin(), transpose_entry.end(), 0);
        // fill off-diagonal, loop all the branches
        for (Idx i = 0; i != n_branch; ++i) {
            // for each branch entry tf and ft, they are transpose to each other
            Idx const entry_1 = off_diag_map[i][0];
            Idx const entry_2 = off_diag_map[i][1];
            transpose_entry[entry_1] = entry_2;
            transpose_entry[entry_2] = entry_1;
        }
    }
};

// See also "Node Admittance Matrix" in "State Estimation Alliander"
template <bool sym>
class YBus {
   public:
    YBus(std::shared_ptr<MathModelTopology const> const& topo_ptr,
         std::shared_ptr<MathModelParam<sym> const> const& param,
         std::shared_ptr<YBusStructure const> const& y_bus_struct = {})
        : math_topology_{topo_ptr} {
        // use existing struct or make new struct
        if (y_bus_struct) {
            y_bus_struct_ = y_bus_struct;
        }
        else {
            y_bus_struct_ = std::make_shared<YBusStructure const>(YBusStructure{*topo_ptr});
        }
        // update values
        update_admittance(param);
    }

    // getter
    Idx size() const {
        return (Idx)bus_entry().size();
    }
    Idx nnz() const {
        return row_indptr().back();
    }
    IdxVector const& row_indptr() const {
        return y_bus_struct_->row_indptr;
    }
    IdxVector const& col_indices() const {
        return y_bus_struct_->col_indices;
    }
    IdxVector const& row_indices() const {
        return y_bus_struct_->row_indices;
    }
    IdxVector const& transpose_entry() const {
        return y_bus_struct_->transpose_entry;
    }
    std::vector<YBusElement> const& y_bus_element() const {
        return y_bus_struct_->y_bus_element;
    }
    IdxVector const& y_bus_entry_indptr() const {
        return y_bus_struct_->y_bus_entry_indptr;
    }
    MathModelTopology const& math_topology() const {
        return *math_topology_;
    }
    MathModelParam<sym> const& math_model_param() const {
        return *math_model_param_;
    }

    ComplexTensorVector<sym> const& admittance() const {
        return *admittance_;
    }
    IdxVector const& bus_entry() const {
        return y_bus_struct_->bus_entry;
    }
    // getter of shared ptr
    std::shared_ptr<IdxVector const> shared_indptr() const {
        return {y_bus_struct_, &y_bus_struct_->row_indptr};
    }
    std::shared_ptr<IdxVector const> shared_indices() const {
        return {y_bus_struct_, &y_bus_struct_->col_indices};
    }
    std::shared_ptr<MathModelTopology const> shared_topology() const {
        return math_topology_;
    }
    std::shared_ptr<YBusStructure const> shared_y_bus_struct() const {
        return y_bus_struct_;
    }

    void update_admittance(std::shared_ptr<MathModelParam<sym> const> const& math_model_param) {
        // overwrite the old cached parameters
        math_model_param_ = math_model_param;
        // construct admittance data
        ComplexTensorVector<sym> admittance(nnz());
        auto const& branch_param = math_model_param_->branch_param;
        auto const& shunt_param = math_model_param_->shunt_param;
        auto const& y_bus_element = y_bus_struct_->y_bus_element;
        auto const& y_bus_entry_indptr = y_bus_struct_->y_bus_entry_indptr;
        // loop for each y bus position
        for (Idx entry = 0; entry != nnz(); ++entry) {
            // start admittance accumulation with zero
            ComplexTensor<sym> entry_admittance{0.0};
            // loop over all entries of this position
            for (Idx element = y_bus_entry_indptr[entry]; element != y_bus_entry_indptr[entry + 1]; ++element) {
                if (y_bus_element[element].element_type == YBusElementType::shunt) {
                    // shunt
                    entry_admittance += shunt_param[y_bus_element[element].idx];
                }
                else {
                    // branch
                    entry_admittance += branch_param[y_bus_element[element].idx]
                                            .value[static_cast<Idx>(y_bus_element[element].element_type)];
                }
            }
            // assign
            admittance[entry] = entry_admittance;
        }
        // move to shared ownership
        admittance_ = std::make_shared<ComplexTensorVector<sym> const>(std::move(admittance));
    }

    ComplexValue<sym> calculate_injection(ComplexValueVector<sym> const& u, Idx bus_number) const {
        Idx const begin = row_indptr()[bus_number];
        Idx const end = row_indptr()[bus_number + 1];
        ComplexValue<sym> const i_inj = std::transform_reduce(
            col_indices().cbegin() + begin, col_indices().cbegin() + end, admittance().cbegin() + begin,
            ComplexValue<sym>{0.0}, std::plus{}, [&u](Idx j, ComplexTensor<sym> const& y) {
                return dot(y, u[j]);
            });
        return conj(i_inj) * u[bus_number];
    }

    ComplexValueVector<sym> calculate_injection(ComplexValueVector<sym> const& u) const {
        ComplexValueVector<sym> s(size());
        std::transform(IdxCount{0}, IdxCount{size()}, s.begin(), [this, &u](Idx bus) {
            return calculate_injection(u, bus);
        });
        return s;
    }

    // calculate branch flow based on voltage
    std::vector<BranchMathOutput<sym>> calculate_branch_flow(ComplexValueVector<sym> const& u) const {
        std::vector<BranchMathOutput<sym>> branch_flow(math_topology_->branch_bus_idx.size());
        std::transform(math_topology_->branch_bus_idx.cbegin(), math_topology_->branch_bus_idx.cend(),
                       math_model_param_->branch_param.cbegin(), branch_flow.begin(),
                       [&u](BranchIdx branch_idx, BranchCalcParam<sym> const& param) {
                           auto const [f, t] = branch_idx;
                           // if one side is disconnected, use zero voltage at that side
                           ComplexValue<sym> const uf = f != -1 ? u[f] : ComplexValue<sym>{0.0};
                           ComplexValue<sym> const ut = t != -1 ? u[t] : ComplexValue<sym>{0.0};
                           BranchMathOutput<sym> output;

                           // See "Branch Flow Calculation" in "State Estimation Alliander"
                           output.i_f = dot(param.yff(), uf) + dot(param.yft(), ut);
                           output.i_t = dot(param.ytf(), uf) + dot(param.ytt(), ut);

                           // See "Shunt Injection Flow Calculation" in "State Estimation Alliander"
                           output.s_f = uf * conj(output.i_f);
                           output.s_t = ut * conj(output.i_t);
                           return output;
                       });
        return branch_flow;
    }

    // calculate shunt flow based on voltage, injection direction
    std::vector<ApplianceMathOutput<sym>> calculate_shunt_flow(ComplexValueVector<sym> const& u) const {
        std::vector<ApplianceMathOutput<sym>> shunt_flow(math_topology_->n_shunt());
        // loop all bus, then all shunt within the bus
        for (Idx bus = 0; bus != size(); ++bus) {
            for (Idx shunt = math_topology_->shunt_bus_indptr[bus]; shunt != math_topology_->shunt_bus_indptr[bus + 1];
                 ++shunt) {
                // See "Branch/Shunt Power Flow" in "State Estimation Alliander"
                // NOTE: the negative sign for injection direction!
                shunt_flow[shunt].i = -dot(math_model_param_->shunt_param[shunt], u[bus]);

                // See "Branch/Shunt Power Flow" in "State Estimation Alliander"
                shunt_flow[shunt].s = u[bus] * conj(shunt_flow[shunt].i);
            }
        }
        return shunt_flow;
    }

   private:
    // csr structure
    std::shared_ptr<YBusStructure const> y_bus_struct_;

    // admittance
    std::shared_ptr<ComplexTensorVector<sym> const> admittance_;

    // cache math topology
    std::shared_ptr<MathModelTopology const> math_topology_;

    // cache the math parameters
    std::shared_ptr<MathModelParam<sym> const> math_model_param_;
};

template class YBus<true>;
template class YBus<false>;

}  // namespace math_model_impl

template <bool sym>
using YBus = math_model_impl::YBus<sym>;

}  // namespace power_grid_model

#endif