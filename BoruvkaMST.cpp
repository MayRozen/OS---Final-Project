#include "BoruvkaMST.hpp"
#include <algorithm>
#include <queue>

// Structure to represent an edge
struct Edge {
    int src, dest;
    double weight;

    Edge(int s, int d, double w) : src(s), dest(d), weight(w) {}
};

// Compare function for sorting edges by weight
struct CompareWeight {
    bool operator()(const Edge& a, const Edge& b) {
        return a.weight < b.weight; // Min-heap based on edge weight
    }
};

Tree BoruvkaMST::computeMST(const Graph& graph) {
    size_t V = static_cast<size_t>(graph.getVertices());
    Tree mst(V); // Initialize an empty Tree for MST
    std::vector<int> cheapest(V, -1); // To store cheapest edge to each component
    std::vector<int> component(V); // To track component of each vertex
    std::vector<bool> inMST(V, false); // To track vertices included in MST
    std::priority_queue<Edge, std::vector<Edge>, CompareWeight> pq;

    // Initialize each vertex as its own component
    for (size_t v = 0; v < V; ++v) {
        component[v] = v;
        for (const auto& neighbor : graph.getAdjList(v)) {
            pq.push(Edge(v, neighbor.first, neighbor.second));
        }
    }

    while (mst.getEdgesCount() < static_cast<int>(V) - 1 && !pq.empty()) {
        Edge e = pq.top();
        pq.pop();

        int u = e.src;
        int v = e.dest;
        double weight = e.weight;

        // Find components of u and v
        int comp_u = find(component, u);
        int comp_v = find(component, v);

        // If they belong to different components, add edge to MST
        if (comp_u != comp_v) {
            mst.addEdge((size_t)u, (size_t)v, weight);
            Union(component, cheapest, u, v, comp_u, comp_v); // Adjust Union call
        }
    }

    return mst;
}

// Helper function to find the component of a vertex using path compression
int BoruvkaMST::find(std::vector<int>& component, int v) {
    if (component[static_cast<size_t>(v)] != v) {
        component[static_cast<size_t>(v)] = find(component, component[static_cast<size_t>(v)]);
    }
    return component[static_cast<size_t>(v)];
}

// Helper function to unify two components using union by rank
void BoruvkaMST::Union(std::vector<int>& component, std::vector<int>& cheapest, int u, int v, int comp_u, int comp_v) {
    if (cheapest[static_cast<size_t>(comp_u)] == -1 || cheapest[static_cast<size_t>(comp_u)] > cheapest[static_cast<size_t>(comp_v)]) {
        cheapest[static_cast<size_t>(comp_u)] = cheapest[static_cast<size_t>(comp_v)];
    }
    component[static_cast<size_t>(comp_v)] = comp_u;
}
