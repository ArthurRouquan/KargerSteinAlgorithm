#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "karger.hpp"


template <typename node_t>
EdgesVectorGraph<node_t> read_col_instance(std::string_view file) {
    EdgesVectorGraph<node_t> graph;
    std::ifstream instance;
    instance.open(file.data());
    if (!instance) throw std::runtime_error("Such instance doesn't exist.");
    for (std::string line; std::getline(instance, line);)
    {
        std::istringstream input(line);
        switch(line[0]) {
            case 'p':
                input.ignore(6);
                std::size_t m;
                input >> graph.n >> m;
                graph.edges.reserve(m);
                break;
            case 'e':
                input.ignore(2);
                Edge<node_t> edge;
                input >> edge.tail >> edge.head;
                --edge.tail; --edge.head; // instance files starting vertex is 1
                graph.edges.push_back(std::move(edge));
                break;
            default:
                break;
        }
    }
    return graph;
}
