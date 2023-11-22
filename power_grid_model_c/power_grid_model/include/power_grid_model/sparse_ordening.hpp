// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRIdx_MODEL_MATH_SOLVER_SPARSE_ORDENING_HPP
#define POWER_GRIdx_MODEL_MATH_SOLVER_SPARSE_ORDENING_HPP

#include "power_grid_model.hpp"

#include <algorithm> // remove and remove_if
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace power_grid_model {

namespace detail {
inline void remove_element_vector_pair(Idx& u, std::vector<std::pair<Idx, Idx>>& dgd) {
    Idx i = 0;
    while (i < dgd.size()) {
        if (dgd[i].first == u) {
            dgd.erase(dgd.begin() + i);
            break;
        } else {
            i++;
        }
    }
}

inline void set_element_vector_pair(Idx& u, Idx v, std::vector<std::pair<Idx, Idx>>& dgd) {
    Idx i = 0;
    while (i < dgd.size()) {
        if (dgd[i].first == u) {
            dgd[i].second = v;
            break;
        } else {
            i++;
        }
    }
}

inline std::vector<Idx> adj(Idx& u, std::map<Idx, std::vector<Idx>>& d) {
    std::vector<Idx> l;

    for (const auto& it : d) {
        if (it.first == u)
            l.insert(l.end(), it.second.begin(), it.second.end());

        if (find(it.second.begin(), it.second.end(), u) != it.second.end()) {
            std::vector<Idx> v{it.first};
            l.insert(l.end(), v.begin(), v.end());
        }
    }

    return l;
}

inline bool compair_ids(std::pair<Idx, Idx>& a, std::pair<Idx, Idx>& b) { return a.first < b.first; }

inline std::vector<std::pair<Idx, std::vector<std::pair<Idx, Idx>>>>
comp_size_degrees_graph(std::map<Idx, std::vector<Idx>>& d) {
    std::vector<std::pair<Idx, Idx>> dd;
    std::vector<Idx> v;
    Idx n = 0;

    for (const auto& it : d) {
        Idx k = it.first;
        if (find(v.begin(), v.end(), k) == v.end()) {
            std::vector<Idx> vk{k};
            v.insert(v.end(), vk.begin(), vk.end());
            n += 1;
            dd.push_back({k, adj(k, d).size()});
        }
        for (const Idx& el : it.second) {
            Idx e = el;
            if (find(v.begin(), v.end(), e) == v.end()) {
                std::vector<Idx> ve{e};
                v.insert(v.end(), ve.begin(), ve.end());
                n += 1;
                dd.push_back({e, adj(e, d).size()});
            }
        }
    }

    sort(dd.begin(), dd.end());

    return {{n, dd}};
}

inline std::vector<std::pair<std::vector<Idx>, std::vector<Idx>>>
check_indistguishable(Idx& u, std::map<Idx, std::vector<Idx>>& d) {
    std::vector<Idx> l, rl, lu, lv, vu{u}, vv;
    l = adj(u, d);
    lu = l;
    lu.insert(lu.end(), vu.begin(), vu.end());

    for (auto& v : l) {
        lv = adj(v, d);
        vv = {v};
        lv.insert(lv.end(), vv.begin(), vv.end());
        sort(lu.begin(), lu.end());
        sort(lv.begin(), lv.end());
        if (lu == lv) {
            rl.insert(rl.end(), vv.begin(), vv.end());
        }
    }

    return {{l, rl}};
}

inline std::map<Idx, std::vector<Idx>> make_clique(std::vector<Idx>& l) {
    std::map<Idx, std::vector<Idx>> d;
    Idx b = l.size() - 1;

    for (int i = 0; i < b; i++) {
        Idx index = i + 1;
        auto start = l.begin() + index;
        std::vector<Idx> sl(l.size() - index);
        copy(start, l.end(), sl.begin());
        d[l[i]] = sl;
    }

    return d;
}

inline bool in_graph(std::pair<Idx, Idx> const& e, std::map<Idx, std::vector<Idx>> const& d) {
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

inline std::vector<Idx> remove_vertices_update_degrees(Idx& u, std::map<Idx, std::vector<Idx>>& d,
                                                       std::vector<std::pair<Idx, Idx>>& dgd,
                                                       std::vector<std::pair<Idx, Idx>>& fills) {
    std::vector<std::pair<std::vector<Idx>, std::vector<Idx>>> nbsrl = check_indistguishable(u, d);
    std::vector<Idx>& nbs = nbsrl[0].first;
    std::vector<Idx>& rl = nbsrl[0].second;
    std::vector<Idx> alpha = rl, vu{u};
    std::map<Idx, std::vector<Idx>> dd;

    rl.insert(rl.begin(), vu.begin(), vu.end());

    for (auto& uu : rl) {
        if (uu != u)
            nbs.erase(remove(nbs.begin(), nbs.end(), uu), nbs.end());

        remove_element_vector_pair(uu, dgd);
        std::vector<Idx> el;
        for (auto& it : d) {
            it.second.erase(remove(it.second.begin(), it.second.end(), uu), it.second.end());
            if (it.second.empty()) {
                Idx k = it.first;
                std::vector<Idx> vk{k};
                el.insert(el.end(), vk.begin(), vk.end());
            }
        }

        std::vector<Idx> vuu{uu};
        el.insert(el.end(), vuu.begin(), vuu.end());

        for (auto& it : el)
            d.erase(it);
    }

    dd = make_clique(nbs);

    for (auto& it : dd) {
        Idx k = it.first;
        for (const Idx& e : it.second) {
            std::pair<Idx, Idx> t{k, e};
            if (!in_graph(t, d)) {
                if (d.find(k) != d.end()) {
                    std::vector<Idx> ve{e};
                    d[k].insert(d[k].end(), ve.begin(), ve.end());
                    fills.push_back({k, e});
                } else if (d.find(e) != d.end()) {
                    std::vector<Idx> vk{k};
                    d[e].insert(d[e].end(), vk.begin(), vk.end());
                    fills.push_back({e, k});
                } else {
                    std::vector<Idx> ve{e};
                    d[k].insert(d[k].end(), ve.begin(), ve.end());
                    fills.push_back({k, e});
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

inline std::pair<std::vector<Idx>, std::vector<std::pair<Idx, Idx>>>
minimum_degree_ordering(std::map<Idx, std::vector<Idx>>& d) {
    std::vector<std::pair<Idx, std::vector<std::pair<Idx, Idx>>>> data = detail::comp_size_degrees_graph(d);
    Idx& n = data[0].first;
    std::vector<std::pair<Idx, Idx>>& dgd = data[0].second;
    std::vector<Idx> alpha;
    std::vector<std::pair<Idx, Idx>> fills;

    for (int k = 0; k < n; k++) {
        Idx u =
            get<0>(*min_element(begin(dgd), end(dgd), [](auto lhs, auto rhs) { return get<1>(lhs) < get<1>(rhs); }));
        std::vector<Idx> vu{u};
        alpha.insert(alpha.end(), vu.begin(), vu.end());
        if ((d.size() == 1) and d.begin()->second.size() == 1) {
            Idx a = d.begin()->first;
            Idx b = d.begin()->second[0];
            if (alpha.back() == a) {
                std::vector<Idx> vb{b};
                alpha.insert(alpha.end(), vb.begin(), vb.end());
            } else {
                std::vector<Idx> va{a};
                alpha.insert(alpha.end(), va.begin(), va.end());
            }
            break;
        } else {
            std::vector<Idx> va = detail::remove_vertices_update_degrees(u, d, dgd, fills);
            alpha.insert(alpha.end(), va.begin(), va.end());
            if (d.empty()) {
                break;
            }
        }
    }
    return {alpha, fills};
}
} // namespace power_grid_model

#endif
