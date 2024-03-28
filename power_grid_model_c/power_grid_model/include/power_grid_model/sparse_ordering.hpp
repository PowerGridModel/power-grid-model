// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common/common.hpp"

#include <algorithm>
#include <map>
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

inline IdxVector adj(Idx const u, std::map<Idx, IdxVector> const& d) {
    IdxVector l;

    for (const auto& [k, adjacent] : d) {
        if (k == u) {
            l.insert(l.end(), adjacent.cbegin(), adjacent.cend());
        } else if (std::ranges::find(adjacent, u) != adjacent.cend()) {
            l.push_back(k);
        }
    }

    return l;
}

inline std::vector<std::pair<Idx, std::vector<std::pair<Idx, Idx>>>>
comp_size_degrees_graph(std::map<Idx, IdxVector> const& d) {
    std::vector<std::pair<Idx, Idx>> dd;
    IdxVector v;
    Idx n = 0;

    for (auto const& [k, adjacent] : d) {
        if (std::ranges::find(v, k) == v.end()) {
            ++n;
            v.push_back(k);
            dd.emplace_back(k, adj(k, d).size());
        }
        for (Idx const e : adjacent) {
            if (std::ranges::find(v, e) == v.end()) {
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
        d[l[i]] = std::move(sl);
    }

    return d;
}

inline std::vector<std::pair<IdxVector, IdxVector>> check_indistguishable(Idx const u,
                                                                          std::map<Idx, IdxVector> const& d) {
    IdxVector rl;

    auto l = adj(u, d);
    auto lu = l;
    lu.push_back(u);

    for (auto const& v : l) {
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
    if (auto edges_it = d.find(e.first);
        edges_it != d.cend() && std::ranges::find(edges_it->second, e.second) != edges_it->second.cend()) {
        return true;
    }
    if (auto edges_it = d.find(e.second);
        edges_it != d.cend() && std::ranges::find(edges_it->second, e.first) != edges_it->second.cend()) {
        return true;
    }
    return false;
}

inline IdxVector remove_vertices_update_degrees(Idx const u, std::map<Idx, IdxVector>& d,
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
        for (auto& [e, adjacent] : d) {
            std::erase(adjacent, uu);
            if (adjacent.empty()) {
                el.push_back(e);
            }
        }

        el.push_back(uu);

        for (auto const& it : el) {
            d.erase(it);
        }
    }

    dd = make_clique(nbs); // TODO check data type

    auto const add_element = [&fills](Idx from, Idx to, IdxVector& from_adjacent) {
        from_adjacent.push_back(to);
        fills.emplace_back(from, to);
    };

    for (auto const& [k, adjacent] : dd) {
        auto it = d.find(k);
        for (Idx const e : adjacent) {
            if (!in_graph(std::make_pair(k, e), d)) {
                if (it != d.end()) {
                    add_element(k, e, it->second);
                } else if (auto e_it = d.find(e); e_it != d.end()) {
                    add_element(e, k, e_it->second);
                } else {
                    std::tie(it, std::ignore) = d.try_emplace(k);
                    add_element(k, e, it->second);
                }
            }
        }
    }

    for (auto const& e : nbs) {
        set_element_vector_pair(e, static_cast<Idx>(adj(e, d).size()), dgd);
    }

    return alpha;
}
} // namespace detail

inline std::pair<IdxVector, std::vector<std::pair<Idx, Idx>>> minimum_degree_ordering(std::map<Idx, IdxVector>& d) {
    auto data = detail::comp_size_degrees_graph(d);
    auto& [n, dgd] = data[0];

    IdxVector alpha;
    std::vector<std::pair<Idx, Idx>> fills;

    for (Idx k = 0; k < n; ++k) {
        Idx const u =
            get<0>(*std::ranges::min_element(dgd, [](auto lhs, auto rhs) { return get<1>(lhs) < get<1>(rhs); }));
        alpha.push_back(u);
        if ((d.size() == 1) && d.begin()->second.size() == 1) {
            Idx const from = d.begin()->first;
            Idx const to = d.begin()->second[0];
            alpha.push_back(alpha.back() == from ? to : from);
            return {alpha, fills};
        }
        std::ranges::copy(detail::remove_vertices_update_degrees(u, d, dgd, fills), std::back_inserter(alpha));
        if (d.empty()) {
            return {alpha, fills};
        }
    }
    return {alpha, fills};
}
} // namespace power_grid_model
