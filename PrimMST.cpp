#include "PrimMST.hpp"
#include <queue>
#include <vector>
#include <limits>

struct Edge {
    int src, dest;
    double weight;

    Edge(int s, int d, double w) : src(s), dest(d), weight(w) {}
};

struct CompareWeight {
    bool operator()(const Edge& a, const Edge& b) {
        return a.weight > b.weight; // Min-heap based on edge weight
    }
};

Tree PrimMST::computeMST(const Graph& graph) {
    size_t V = (size_t)graph.getVertices();
    Tree mst(V); // Initialize an empty Tree for MST
    std::vector<bool> inMST(V, false); // To track vertices included in MST
    std::priority_queue<Edge, std::vector<Edge>, CompareWeight> pq;

    // Start with vertex 0 (assuming graph is connected)
    size_t startVertex = 0;
    inMST[startVertex] = true;

    // Add all edges from startVertex to the priority queue
    for (const auto& neighbor : graph.getAdjList(startVertex)) {
        pq.push(Edge(startVertex, neighbor.first, neighbor.second));
    }

    while (!pq.empty()) {
        Edge e = pq.top();
        pq.pop();

        size_t u = (size_t)e.src;
        size_t v = (size_t)e.dest;
        double weight = e.weight;

        // Check for cycle (ignore if both vertices are already in MST)
        if (inMST[u] && inMST[v])
            continue;

        // Add the current edge to MST
        mst.addEdge(u, v, weight);
        if (!inMST[u]) {
            inMST[u] = true;
            for (const auto& neighbor : graph.getAdjList(u)) {
                if (!inMST[(size_t)neighbor.first])
                    pq.push(Edge(u, neighbor.first, neighbor.second));
            }
        }
        if (!inMST[v]) {
            inMST[v] = true;
            for (const auto& neighbor : graph.getAdjList(v)) {
                if (!inMST[(size_t)neighbor.first])
                    pq.push(Edge(v, neighbor.first, neighbor.second));
            }
        }
    }

    return mst;
}
