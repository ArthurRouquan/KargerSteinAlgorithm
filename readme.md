# Karger's algorithm and Karger–Stein algorithm in Modern C++20

Implementation in C++20 of [Karger's algorithm](https://en.wikipedia.org/wiki/Karger%27s_algorithm) using an [Union-Find data structure](https://en.wikipedia.org/wiki/Disjoint-set_data_structure) to keep track of merged nodes in O(mα(n) + n) and one of its extension, the Karger–Stein algorithm in O((mα(n) + n) log(n)). This implementation was made for educational purposes.

## Oversight
### Data structures
* `Edge` is just a pair of integers (of `node_t` type) that represents an (directed/undirected) edge.
* `EdgesVectorGraph` represents a graph as a simple set of edges. It is assumed that the vertex indices of the edges are between 0 and n - 1 (included).
* `GraphCut` stores the ouput cut of the algorithms. For performance purposes, we delay the computation of the vertices in the two partitions after the best minimum cut is found.
* `ContractedGraph` is an extension of `EdgesVectorGraph` with an Union-Find data structure to keep track of merged vertices. It is used as an intermediate graph in the Karger–Stein algorithm.
### Algorithms
* `karger_union_find` randomly contracts the edges of the given graph until it has two vertices, from there we compute the size of this cut. The graph isn't per se modifed, only its vector of edges is shuffled.
* `karger_stein_union_find` implements the recursive aspect of the Karger–Stein algorithm with a stack of graphs to contract.

### Main
* `minimal_example` provides a minimal... example.
* `main` essentially reads the given graph instance file and sends it to the algorithms.

## How to run it?

This project use CMake. It's an overkill. To run the executable you need to pass a graph instance .col file:
```
$ krager ..\graph_instances\le450_25d.col

Input graph: "..\graph_instances\le450_25d.col" (|V| = 450,  |E| = 17425)

Algorithm: "Karger"
    - Number of repetitions: 617186
    - Best minimum cut's size found: 11
    - Duration: 30280ms

Algorithm: "Karger-Stein"
    - Number of repetitions: 37        
    - Best minimum cut's size found: 11
    - Duration: 21369ms
```

## References
1. https://en.wikipedia.org/wiki/Karger%27s_algorithm
2. David R. Karger, Clifford Stein, *A New Approach to the Minimum Cut Problem* https://doi.org/10.1145%2F234533.234534 (1996) (http://www.columbia.edu/~cs2035/courses/ieor6614.S09/Contraction.pdf)