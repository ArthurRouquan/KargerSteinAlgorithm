#pragma once

#include <algorithm>
#include <cmath>
#include <optional>
#include <random>
#include <ranges>
#include <stack>
#include <vector>


auto& prng_engine() {
    thread_local static std::mt19937 engine{std::random_device{}()};
    return engine;
} 


template <typename node_t>
struct Edge { node_t tail, head; };

/* Represents a graph with n vertices as a collection of edges whose vertices are indexed between 0
and n - 1. */
template <typename node_t>
struct EdgesVectorGraph
{
    node_t n; // number of vertices
    std::vector<Edge<node_t>> edges;
};


/* An Union-Find data structure (https://fr.wikipedia.org/wiki/Union-Find) is a data structure that
stores a partition of a set into disjoint subsets. Tree height is controlled using union by size.
Use path compression technique. */
template <typename T>
struct UnionFind
{
    struct Subset { T id, size; };
    std::vector<Subset> subsets;
    T nb_subsets;

    UnionFind(T n) : nb_subsets{n} { 
        subsets.reserve(n);
        for (T i = 0; i < n; ++i)
            subsets.emplace_back(i, 1);
    }

    auto find(T x) {
        auto root = x;
        while (root != subsets[root].id) { root = subsets[root].id; }
        while (subsets[x].id != root) { auto id = subsets[x].id; subsets[x].id = root; x = id; }
        return root;
    }

    void merge(T x, T y) {
        auto const i = find(x); auto const j = find(y);
        if (i == j) return;
        if (subsets[i].size < subsets[j].size) { subsets[i].id = j; subsets[j].size += subsets[i].size; }
        else                                   { subsets[j].id = i; subsets[i].size += subsets[j].size; }
        --nb_subsets;
    }

    bool connected(T x, T y) { return find(x) == find(y); }
};


/* A data structure representing a cut of a graph. */
template <typename node_t>
struct GraphCut
{
    std::size_t cut_size;
    UnionFind<node_t> uf; // used to identify the two partitions of nodes

    bool operator<(GraphCut const& other) const { return cut_size < other.cut_size; }

    auto get_partitions() const {
        std::vector<node_t> P, Q;
        P.reserve(uf.subsets[0].size); Q.reserve(std::size(uf.subsets) - std::size(P));
        for (node_t i = 0; i < std::size(uf.subsets); ++i)
            uf.subsets[i].id == uf.subsets[0].id ? P.push_back(i) : Q.push_back(i);
        return std::array{P, Q};
    }
};


/* Karger's contraction algorithm in O(n + mα(n)) using an Union-Find data structure to keep track
of merged vertices. The graph is assumed to be connected and nodes indexed between 0 and n-1. Repeat
this function C(n,2)*log(n) = n*(n-1)/2*log(n) for high probability of obtaining the minimum global
cut. The graph isn't per se modifed, only its vector of edges is shuffled. */
template <typename node_t>
GraphCut<node_t> karger_union_find(EdgesVectorGraph<node_t>& graph)
{
    auto& mt = prng_engine();
    UnionFind uf{graph.n};
    auto start = begin(graph.edges);
    for (int m = size(graph.edges) - 1; uf.nb_subsets != 2; ++start, --m) {
        std::iter_swap(start, start + std::uniform_int_distribution<>{0, m}(mt));
        uf.merge(start->tail, start->head);
    }
    return {(std::size_t) std::count_if(start, end(graph.edges), [&](auto e)
        { return !uf.connected(e.tail, e.head); }), std::move(uf)};
}


/* Kargen-Stein's contraction recursive algorithm. Instead of using a straighforward recursion, we
keep the intermediate graphs to contract in a stack. Repeat this function log²(n) for high probabili
-ty of obtaining the minimum global cut. */
template <typename node_t>
auto karger_stein_union_find(EdgesVectorGraph<node_t> const& input_graph)
{
    /* A data structure to hold an intermediate contracted graph state. The Union-Find structure
    is used to keep track of the merged nodes. */ 
    struct ContractedGraph : EdgesVectorGraph<node_t> { UnionFind<node_t> uf; };

    /* Contracts the given graph until it has nb_vertices vertices. The graph isn't per se modifed,
    only its vector of edges is shuffled. */
    auto contract = [&mt = prng_engine()](ContractedGraph& graph, node_t nb_vertices) {   
        UnionFind uf{graph.uf};
        auto start = begin(graph.edges);
        for (int m = size(graph.edges) - 1; uf.nb_subsets != nb_vertices; ++start, --m) {
            std::iter_swap(start, start + std::uniform_int_distribution<>{0, m}(mt));
            uf.merge(start->tail, start->head);
        }
        decltype(graph.edges) edges;
        edges.reserve(end(graph.edges) - start);
        std::copy_if(start, end(graph.edges), std::back_inserter(edges),
            [&](auto e){ return !uf.connected(e.tail, e.head); }); // remove self-loops
        return ContractedGraph{nb_vertices, std::move(edges), std::move(uf)};
    };

    /* Returns the cut represented by an intermediate contracted graph. The given graph is supposed
    to have no self-loops and to have two nodes (components). */
    auto cut = [](ContractedGraph&& graph){ return GraphCut{std::size(graph.edges), graph.uf}; };
    
    constexpr double INV_SQRT_2 = 1.0 / std::sqrt(2);
    GraphCut<node_t> best_minimum_cut{input_graph.n, {{}}};
    std::stack<ContractedGraph, std::vector<ContractedGraph>> graphs;
    graphs.push({input_graph.n, input_graph.edges, {input_graph.n}});

    while (!graphs.empty()) // algorithm's main loop
    {
        auto graph = graphs.top();
        graphs.pop();

        if (graph.n <= 6) {
            best_minimum_cut = std::min(best_minimum_cut, cut(contract(graph, 2)));
        } else {
            node_t t = 1 + std::ceil(graph.n * INV_SQRT_2);
            graphs.push(contract(graph, t));
            graphs.push(contract(graph, t));
        }
    }

    return best_minimum_cut;
}
