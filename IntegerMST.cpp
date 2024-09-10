#include "IntegerMST.hpp"
#include <algorithm> // For std::sort
#include <queue>     // For priority_queue
#include <vector>    // For vector
#include <cstddef>  // For size_t

// Define IntegerEdge and CompareIntegerEdge if not already done
struct IntegerEdge {
    size_t src, dest;
    int weight;

    IntegerEdge(size_t u, size_t v, int w) : src(u), dest(v), weight(w) {}
};

// Comparator for priority queue (min-heap based on weight)
struct CompareIntegerEdge {
    bool operator()(const IntegerEdge& a, const IntegerEdge& b) {
        return a.weight > b.weight; // Min-heap for edges with lower weight having higher priority
    }
};

// Utility function to find the set of an element u (uses path compression)
size_t IntegerMST::find(std::vector<size_t>& parent, size_t u) {
    if (parent[u] != u) {
        parent[u] = find(parent, parent[u]);
    }
    return parent[u];
}

// Utility function to union two sets u and v (uses union by rank)
void IntegerMST::Union(std::vector<size_t>& parent, std::vector<size_t>& rank, size_t u, size_t v) {
    size_t rootU = find(parent, u);
    size_t rootV = find(parent, v);

    if (rootU != rootV) {
        if (rank[rootU] < rank[rootV]) {
            parent[rootU] = rootV;
        } else if (rank[rootU] > rank[rootV]) {
            parent[rootV] = rootU;
        } else {
            parent[rootV] = rootU;
            rank[rootU]++;
        }
    }
}

// Main function to compute the MST using a priority queue and adjacency list (similar to Prim's algorithm)
Tree IntegerMST::computeMST(const Graph& graph) {
    size_t V = (size_t)graph.getVertices();
    Tree mst(V); // Initialize an empty Tree for MST
    std::priority_queue<IntegerEdge, std::vector<IntegerEdge>, CompareIntegerEdge> pq; // Min-heap for edges

    std::vector<size_t> parent(V);
    std::vector<size_t> rank(V, 0);
    std::vector<bool> inMST(V, false); // Track vertices included in MST
    for (size_t i = 0; i < V; ++i) {
        parent[i] = i;
    }

    // Add all edges starting from vertex 0 to the priority queue
    for (const auto& neighbor : graph.getAdjList(0)) { // Assuming getAdjList returns neighbors of a vertex
        pq.push(IntegerEdge(0, (size_t)neighbor.first, (size_t)neighbor.second));
    }

    int mst_weight = 0;

    while (!pq.empty()) {
        IntegerEdge edge = pq.top();
        pq.pop();

        size_t u = find(parent, edge.src);
        size_t v = find(parent, edge.dest);

        // If u and v are in different sets, include the edge in the MST
        if (u != v) {
            mst.addEdge(edge.src, edge.dest, edge.weight);
            mst_weight += edge.weight;
            Union(parent, rank, u, v);

            // Add all adjacent edges of the new node to the priority queue
            for (const auto& neighbor : graph.getAdjList(edge.dest)) {
                if (!inMST[(size_t)neighbor.first]) {
                    pq.push(IntegerEdge(edge.dest, (size_t)neighbor.first, (size_t)neighbor.second));
                }
            }
        }
    }

    return mst;
}
