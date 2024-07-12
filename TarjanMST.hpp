#ifndef TARJAN_MST_HPP
#define TARJAN_MST_HPP

#include "MSTStrategy.hpp"
#include "Graph.hpp" // Assuming Graph class is defined elsewhere
#include <vector>
#include <stack> // Include for std::stack

// Structure to represent an edge
struct Edge {
    int src, dest;
    double weight;

    Edge(int s, int d, double w) : src(s), dest(d), weight(w) {}
};

class TarjanMST : public MSTStrategy {
public:
    virtual Tree computeMST(const Graph& graph) override;
    void DFS(const Graph& graph, int u, std::vector<bool>& visited, std::stack<Edge>& edges);

private:
    int find(std::vector<int>& parent, int u);
    void Union(std::vector<int>& parent, std::vector<int>& rank, int u, int v);
};

#endif // TARJAN_MST_HPP
