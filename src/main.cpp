#include <cstdint>
#include <iostream>
#include <functional>
#include <array>
#include <chrono>

#include "karger.hpp"
#include "instance_reader.hpp"

/* http://www.columbia.edu/~cs2035/courses/ieor6614.S09/Contraction.pdf (https://doi.org/10.1145%2F234533.234534) */


void minimal_example()
{
    EdgesVectorGraph<int> graph{8, {
        {0, 1}, {0, 2}, {0, 3}, {1, 2}, {1, 3}, {2, 3}, {4, 5},
        {4, 6}, {4, 7}, {5, 6}, {5, 7}, {6, 7}, {1, 4}, {3, 4}
    }};

    auto cut = karger_union_find(graph); // or karger_stein_union_find(graph)

    std::cout << "Cut's size: " << cut.cut_size << '\n';

    std::cout << "Partitions: ";
    for (auto const& partition : cut.get_partitions()) {
        std::cout << "{ ";
        for (auto u : partition) std::cout << u << ' ';
        std::cout << "} ";
    }
}


int main(int argc, char* argv[])
{
    using node_t = std::uint32_t;
    if (argc != 2) throw std::runtime_error("No input file.");
    auto graph = read_col_instance<node_t>(argv[1]);

    std::cout << "\nInput graph: \"" << argv[1] << "\" (|V| = " << graph.n << ", |E| = "
        << std::size(graph.edges) << ")\n";

    struct MinimumCutAlgorithm {
        std::string name;
        std::function<GraphCut<node_t>(EdgesVectorGraph<node_t>&)> algorithm;
        std::size_t nb_repeat;
        auto operator()(EdgesVectorGraph<node_t>& graph) const { return algorithm(graph); }
    };

    std::array<MinimumCutAlgorithm, 2> algorithms{{
        {"Karger",       karger_union_find<node_t>,       static_cast<std::size_t>(0.5 * graph.n * (graph.n - 1) * std::log(graph.n))},
        {"Karger-Stein", karger_stein_union_find<node_t>, static_cast<std::size_t>(std::log(graph.n) * std::log(graph.n))}
    }}; 

    for (auto const& algorithm : algorithms)
    {
        std::cout << "\nAlgorithm: \"" << algorithm.name << "\"\n"
                  << "    - Number of repetitions: " << algorithm.nb_repeat << '\n';
        GraphCut<node_t> best_minimum_cut{graph.n, {{}}};
        auto time_start{std::chrono::steady_clock::now()};

        for(std::size_t i = algorithm.nb_repeat; i; --i)
            best_minimum_cut = std::min(best_minimum_cut, algorithm(graph));
        
        auto duration = duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - time_start).count();
        std::cout << "    - Best minimum cut's size found: " << best_minimum_cut.cut_size
                  << "\n    - Duration: " << duration << "ms\n";
    }
    
    std::cout << "\n";
}