// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common/common.hpp"

#include <algorithm>
#include <cassert>
#include <compare>
#include <map>
#include <set>
#include <utility>
#include <vector>

namespace power_grid_model {

namespace detail {
class DegreeLookup {
  public:
    void set(Idx u, Idx degree) {
        if (auto it = vertex_to_degree.find(u); it != vertex_to_degree.end()) {
            degrees_to_vertex[it->second].erase(u);
            it->second = degree;
        } else {
            vertex_to_degree.try_emplace(u, degree);
        }
        degrees_to_vertex[degree].insert(u);
    }

    void erase(Idx u) {
        auto vertex_it = vertex_to_degree.find(u);
        if (vertex_it == vertex_to_degree.end()) {
            return;
        }
        Idx const degree = vertex_it->second;
        vertex_to_degree.erase(vertex_it);

        if (auto degree_it = degrees_to_vertex.find(degree); degree_it != degrees_to_vertex.end()) {
            degree_it->second.erase(u);
            if (degree_it->second.empty()) {
                degrees_to_vertex.erase(degree_it);
            }
        }
    }

    friend auto min_element(DegreeLookup const& dgd);

  private:
    std::map<Idx, std::set<Idx>> degrees_to_vertex;
    std::map<Idx, Idx> vertex_to_degree;
};

inline auto min_element(DegreeLookup const& dgd) {
    Idx const vertex = *dgd.degrees_to_vertex.begin()->second.begin();
    return dgd.vertex_to_degree.find(vertex);
}

inline void remove_element_degree(Idx u, DegreeLookup& dgd) { dgd.erase(u); }

inline void set_element_degree(Idx u, Idx degree, DegreeLookup& dgd) { dgd.set(u, degree); }

inline Idx num_adjacent(Idx const u, std::map<Idx, IdxVector> const& d) {
    if (auto it = d.find(u); it != d.end()) {
        return static_cast<Idx>(it->second.size());
    }
    return 0;
}

inline IdxVector const& adj(Idx const u, std::map<Idx, IdxVector> const& d) { return d.at(u); }

inline std::vector<std::pair<Idx, DegreeLookup>> comp_size_degrees_graph(std::map<Idx, IdxVector> const& d) {
    DegreeLookup dd;
    IdxVector v;

    for (auto const& [k, adjacent] : d) {
        v.push_back(k);
        set_element_degree(k, adjacent.size(), dd);
    }

    return {{d.size(), dd}};
}

inline std::map<Idx, IdxVector> make_clique(IdxVector& l) {
    std::map<Idx, IdxVector> d;

    for (Idx i = 0; i < static_cast<Idx>(l.size()); i++) {
        IdxVector sl(l.size() - 1);
        std::copy(l.begin(), l.begin() + i, sl.begin());
        std::copy(l.begin() + i + 1, l.end(), sl.begin() + i);
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
    std::ranges::sort(lu);

    for (auto const& v : l) {
        auto lv = adj(v, d);
        lv.push_back(v);
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
    return false;
}

inline IdxVector remove_vertices_update_degrees(Idx const u, std::map<Idx, IdxVector>& d, DegreeLookup& dgd,
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

        remove_element_degree(uu, dgd);
        IdxVector el;
        for (auto& e : d[uu]) {
            auto& adjacents = d[e];
            std::erase(adjacents, uu);
            if (adjacents.empty()) {
                el.push_back(e);
            }
        }

        el.push_back(uu);

        for (auto const& it : el) {
            d.erase(it);
        }
    }

    dd = make_clique(nbs);

    for (auto const& [k, adjacent] : dd) {
        auto it = d.find(k);
        for (Idx const e : adjacent) {
            if (!in_graph(std::make_pair(k, e), d)) {
                if (it == d.end()) {
                    std::tie(it, std::ignore) = d.try_emplace(k);
                }
                it->second.push_back(e);
                d[e].push_back(k);
                fills.emplace_back(k, e);
            }
        }
    }

    for (auto const& e : nbs) {
        set_element_degree(e, num_adjacent(e, d), dgd);
    }

    return alpha;
}
} // namespace detail

inline std::pair<IdxVector, std::vector<std::pair<Idx, Idx>>> minimum_degree_ordering(std::map<Idx, IdxVector> d_) {
    // make symmetric
    auto d = d_;
    for (auto& [k, adjacent] : d_) {
        for (auto a : adjacent) {
            d[a].push_back(k);
        }
    }
    for (auto& [k, adjacent] : d) {
        std::ranges::sort(adjacent);
    }

    auto data = detail::comp_size_degrees_graph(d);
    auto& [n, dgd] = data[0];

    IdxVector alpha;
    std::vector<std::pair<Idx, Idx>> fills;

    for (Idx k = 0; k < n; ++k) {
        Idx const u = get<0>(*detail::min_element(dgd));
        alpha.push_back(u);
        if (d.size() == 2) {
            assert(d.begin()->second.size() == 1);

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