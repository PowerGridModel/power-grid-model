// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_MATH_SOLVER_SPARSE_ORDENING_HPP
#define POWER_GRID_MODEL_MATH_SOLVER_SPARSE_ORDENING_HPP

#include "power_grid_model.hpp"

#include <algorithm> // remove and remove_if
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace power_grid_model {

namespace detail {
inline void remove_element_vector_pair(Idx u, std::vector<std::pair<Idx, Idx>>& dgd) {
    std::erase_if(dgd, [u](auto const& v) { return v.first == u; });
}

inline void set_element_vector_pair(Idx u, Idx v, std::vector<std::pair<Idx, Idx>>& dgd) {
    if (auto it = std::ranges::find_if(dgd, [u](auto const& value) { return value.first == u; }); it != dgd.end()) {
        it->second = v;
    }
}

inline IdxVector adj(Idx& u, std::map<Idx, IdxVector>& d) {
    IdxVector l;

    for (const auto& it : d) {
        if (it.first == u) {
            l.insert(l.end(), it.second.cbegin(), it.second.cend());
        }

        if (std::ranges::find(it.second, u) != it.second.cend()) {
            l.push_back(it.first);
        }
    }

    return l;
}

inline std::vector<std::pair<Idx, std::vector<std::pair<Idx, Idx>>>>
comp_size_degrees_graph(std::map<Idx, IdxVector>& d) {
    std::vector<std::pair<Idx, Idx>> dd;
    IdxVector v;
    Idx n = 0;

    for (const auto& it : d) {
        Idx k = it.first;
        if (std::ranges::find(v, k) == v.end()) {
            ++n;
            v.push_back(k);
            dd.emplace_back(k, adj(k, d).size());
        }
        for (const Idx& el : it.second) {
            Idx e = el;
            if (find(v.begin(), v.end(), e) == v.end()) {
                ++n;
                v.push_back(e);
                dd.emplace_back(e, adj(e, d).size());
            }
        }
    }

    std::ranges::sort(dd);

    return {{n, dd}};
}

inline std::map<Idx, IdxVector> make_clique(IdxVector& l) {
    std::map<Idx, IdxVector> d;

    for (Idx i = 0; i < static_cast<Idx>(l.size()) - 1; i++) {
        Idx const idx = i + 1;
        IdxVector sl(l.size() - idx);
        std::copy(l.begin() + idx, l.end(), sl.begin());
        d[l[i]] = sl;
    }

    return d;
}

inline std::vector<std::pair<IdxVector, IdxVector>> check_indistguishable(Idx& u, std::map<Idx, IdxVector>& d) {
    IdxVector rl;

    auto l = adj(u, d);
    auto lu = l;
    lu.push_back(u);

    for (auto& v : l) {
        auto lv = adj(v, d);
        lv.push_back(v);
        std::ranges::sort(lu);
        std::ranges::sort(lv);
        if (lu == lv) {
            rl.push_back(v);
        }
    }

    return {{l, rl}};
}

inline bool in_graph(std::pair<Idx, Idx> const& e, std::map<Idx, IdxVector> const& d) {
    if (auto edges_it = d.find(e.first); edges_it != d.cend()) {
        if (std::ranges::find(edges_it->second, e.second) != edges_it->second.cend()) {
            return true;
        }
    }
    if (auto edges_it = d.find(e.second); edges_it != d.cend()) {
        if (std::ranges::find(edges_it->second, e.first) != edges_it->second.cend()) {
            return true;
        }
    }
    return false;
}

inline IdxVector remove_vertices_update_degrees(Idx& u, std::map<Idx, IdxVector>& d,
                                                std::vector<std::pair<Idx, Idx>>& dgd,
                                                std::vector<std::pair<Idx, Idx>>& fills) {
    std::vector<std::pair<IdxVector, IdxVector>> nbsrl = check_indistguishable(u, d);
    auto& [nbs, rl] = nbsrl[0];
    IdxVector alpha = rl;
    std::map<Idx, IdxVector> dd;

    rl.push_back(u);

    for (auto uu : rl) {
        if (uu != u) {
            std::erase(nbs, uu);
        }

        remove_element_vector_pair(uu, dgd);
        IdxVector el;
        for (auto& it : d) {
            std::erase(it.second, uu);
            if (it.second.empty()) {
                el.push_back(it.first);
            }
        }

        el.push_back(uu);

        for (auto& it : el) {
            d.erase(it);
        }
    }

    dd = make_clique(nbs);

    for (auto& it : dd) {
        Idx k = it.first;
        for (const Idx& e : it.second) {
            std::pair<Idx, Idx> t{k, e};
            if (!in_graph(t, d)) {
                if (d.find(k) != d.end() || d.find(e) == d.end()) {
                    d[k].push_back(e);
                    fills.emplace_back(k, e);
                } else {
                    d[e].push_back(k);
                    fills.emplace_back(e, k);
                }
            }
        }
    }

    for (auto& e : nbs) {
        set_element_vector_pair(e, adj(e, d).size(), dgd);
    }

    return alpha;
}
} // namespace detail

inline std::pair<IdxVector, std::vector<std::pair<Idx, Idx>>> minimum_degree_ordering(std::map<Idx, IdxVector>& d) {
    auto data = detail::comp_size_degrees_graph(d);
    auto& [n, dgd] = data[0];

    IdxVector alpha;
    std::vector<std::pair<Idx, Idx>> fills;

    for (Idx k = 0; k < n; k++) {
        Idx u = get<0>(*std::ranges::min_element(dgd, [](auto lhs, auto rhs) { return get<1>(lhs) < get<1>(rhs); }));
        alpha.push_back(u);
        if ((d.size() == 1) && d.begin()->second.size() == 1) {
            Idx const a = d.begin()->first;
            Idx const b = d.begin()->second[0];
            alpha.push_back(alpha.back() == a ? b : a);
            break;
        } else {
            std::ranges::copy(detail::remove_vertices_update_degrees(u, d, dgd, fills), std::back_inserter(alpha));
            if (d.empty()) {
                break;
            }
        }
    }
    return {alpha, fills};
}
} // namespace power_grid_model

#endif
