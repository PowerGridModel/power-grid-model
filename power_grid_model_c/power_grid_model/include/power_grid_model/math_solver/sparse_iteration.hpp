// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_MATH_SOLVER_SPARSE_ITERATION_HPP
#define POWER_GRID_MODEL_MATH_SOLVER_SPARSE_ITERATION_HPP

#include "../power_grid_model.hpp"

#include <algorithm> // remove and remove_if
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace power_grid_model::math_solver {

void rmElemVectPair(ID& u, std::vector<std::pair<ID, ID>>& dgd) {
    ID i = 0;
    while (i < dgd.size())
        if (dgd[i].first == u) {
            dgd.erase(dgd.begin() + i);
            break;
        } else
            i++;
}

void setElemVectPair(ID& u, ID v, std::vector<std::pair<ID, ID>>& dgd) {
    ID i = 0;
    while (i < dgd.size()) {
        if (dgd[i].first == u) {
            dgd[i].second = v;
            break;
        } else {
            i++;
        }
    }
}

std::vector<ID> adj(ID& u, std::map<ID, std::vector<ID>>& d) {
    std::vector<ID> l;

    for (const auto& it : d) {
        if (it.first == u)
            l.insert(l.end(), it.second.begin(), it.second.end());

        if (find(it.second.begin(), it.second.end(), u) != it.second.end()) {
            std::vector<ID> v{it.first};
            l.insert(l.end(), v.begin(), v.end());
        }
    }

    return l;
}

bool cmpFirts(std::pair<ID, ID>& a, std::pair<ID, ID>& b) { return a.first < b.first; }

std::vector<std::pair<ID, std::vector<std::pair<ID, ID>>>> compSizeDegreesGraph(std::map<ID, std::vector<ID>>& d) {
    std::vector<std::pair<ID, ID>> dd;
    std::vector<ID> v;
    ID n = 0;

    for (const auto& it : d) {
        ID k = it.first;
        if (find(v.begin(), v.end(), k) == v.end()) {
            std::vector<ID> vk{k};
            v.insert(v.end(), vk.begin(), vk.end());
            n += 1;
            dd.push_back({k, adj(k, d).size()});
        }
        for (const ID& el : it.second) {
            ID e = el;
            if (find(v.begin(), v.end(), e) == v.end()) {
                std::vector<ID> ve{e};
                v.insert(v.end(), ve.begin(), ve.end());
                n += 1;
                dd.push_back({e, adj(e, d).size()});
            }
        }
    }

    sort(dd.begin(), dd.end(), cmpFirts);

    return {{n, dd}};
}

std::vector<std::pair<std::vector<ID>, std::vector<ID>>> checkIndistguishable(ID& u, std::map<ID, std::vector<ID>>& d) {
    std::vector<ID> l, rl, lu, lv, vu{u}, vv;
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

std::map<ID, std::vector<ID>> makeClique(std::vector<ID>& l) {
    std::map<ID, std::vector<ID>> d;
    ID b = l.size() - 1;

    for (int i = 0; i < b; i++) {
        ID index = i + 1;
        auto start = l.begin() + index;
        std::vector<ID> sl(l.size() - index);
        copy(start, l.end(), sl.begin());
        d[l[i]] = sl;
    }

    return d;
}

bool inGraph(std::vector<ID>& e, std::map<ID, std::vector<ID>>& d) {
    bool t1 = (d.find(e[0]) != d.end()) and (find(d[e[0]].begin(), d[e[0]].end(), e[1]) != d[e[0]].end());
    bool t2 = (d.find(e[1]) != d.end()) and (find(d[e[1]].begin(), d[e[1]].end(), e[0]) != d[e[1]].end());

    return (t1 || t2);
}

std::vector<ID> rmvVerticesUpdateDegrees(ID& u, std::map<ID, std::vector<ID>>& d, std::vector<std::pair<ID, ID>>& dgd,
                                         std::vector<std::pair<ID, ID>>& fills) {
    std::vector<std::pair<std::vector<ID>, std::vector<ID>>> nbsrl = checkIndistguishable(u, d);
    std::vector<ID>& nbs = nbsrl[0].first;
    std::vector<ID>& rl = nbsrl[0].second;
    std::vector<ID> alpha = rl, vu{u};
    std::map<ID, std::vector<ID>> dd;

    rl.insert(rl.begin(), vu.begin(), vu.end());

    for (auto& uu : rl) {
        if (uu != u)
            nbs.erase(remove(nbs.begin(), nbs.end(), uu), nbs.end());

        rmElemVectPair(uu, dgd);
        std::vector<ID> el;
        for (auto& it : d) {
            it.second.erase(remove(it.second.begin(), it.second.end(), uu), it.second.end());
            if (it.second.empty()) {
                ID k = it.first;
                std::vector<ID> vk{k};
                el.insert(el.end(), vk.begin(), vk.end());
            }
        }

        std::vector<ID> vuu{uu};
        el.insert(el.end(), vuu.begin(), vuu.end());

        for (auto& it : el)
            d.erase(it);
    }

    dd = makeClique(nbs);

    for (auto& it : dd) {
        ID k = it.first;
        for (const ID& e : it.second) {
            std::vector<ID> t{k, e};
            if (not inGraph(t, d)) {
                if (d.find(k) != d.end()) {
                    std::vector<ID> ve{e};
                    d[k].insert(d[k].end(), ve.begin(), ve.end());
                    fills.push_back({k, e});
                } else if (d.find(e) != d.end()) {
                    std::vector<ID> vk{k};
                    d[e].insert(d[e].end(), vk.begin(), vk.end());
                    fills.push_back({e, k});
                } else {
                    std::vector<ID> ve{e};
                    d[k].insert(d[k].end(), ve.begin(), ve.end());
                    fills.push_back({k, e});
                }
            }
        }
    }

    for (auto& e : nbs) {
        setElemVectPair(e, adj(e, d).size(), dgd);
    }

    return alpha;
}

std::vector<std::pair<std::vector<ID>, std::vector<std::pair<ID, ID>>>>
minimumDegreeAlgorithm(std::map<ID, std::vector<ID>>& d) {
    std::vector<std::pair<ID, std::vector<std::pair<ID, ID>>>> data = compSizeDegreesGraph(d);
    ID& n = data[0].first;
    std::vector<std::pair<ID, ID>>& dgd = data[0].second;
    std::vector<ID> alpha;
    std::vector<std::pair<ID, ID>> fills;
    std::vector<std::pair<std::vector<ID>, std::vector<std::pair<ID, ID>>>> alpha_fills;

    for (int k = 0; k < n; k++) {
        ID u = get<0>(*min_element(begin(dgd), end(dgd), [](auto lhs, auto rhs) { return get<1>(lhs) < get<1>(rhs); }));
        std::vector<ID> vu{u};
        alpha.insert(alpha.end(), vu.begin(), vu.end());
        if ((d.size() == 1) and d.begin()->second.size() == 1) {
            ID a = d.begin()->first;
            ID b = d.begin()->second[0];
            if (alpha.back() == a) {
                std::vector<ID> vb{b};
                alpha.insert(alpha.end(), vb.begin(), vb.end());
            } else {
                std::vector<ID> va{a};
                alpha.insert(alpha.end(), va.begin(), va.end());
            }
            break;
        } else {
            std::vector<ID> va = rmvVerticesUpdateDegrees(u, d, dgd, fills);
            alpha.insert(alpha.end(), va.begin(), va.end());
            if (d.empty()) {
                break;
            }
        }
    }
    alpha_fills = {{alpha, fills}};
    return alpha_fills;
}
} // namespace power_grid_model::math_solver

#endif
