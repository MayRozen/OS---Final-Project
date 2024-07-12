#include "TarjanMST.hpp"
#include "Graph.hpp" // Include Graph header if not included elsewhere
#include <algorithm> // Include for std::sort
#include <vector>
#include <stack>

void TarjanMST::DFS(const Graph& graph, int u, std::vector<bool>& visited, std::stack<Edge>& edges) {
    visited[(size_t)u] = true;

    for (const auto& neighbor : graph.getAdjList((size_t)u)) {
        size_t v = (size_t)neighbor.first;
        double weight = neighbor.second;
        if (!visited[v]) {
            edges.push(Edge(u, v, weight));
            DFS(graph, v, visited, edges);
        }
    }
}

Tree TarjanMST::computeMST(const Graph& graph) {
    size_t V = (size_t)graph.getVertices();
    Tree mst(V); // Initialize an empty Tree for MST
    std::vector<bool> visited(V, false);
    std::stack<Edge> edges;

    // Start DFS from vertex 0 (assuming graph is connected)
    DFS(graph, 0, visited, edges);

    // Sort edges in increasing order on basis of cost
    std::vector<Edge> sortedEdges;
    while (!edges.empty()) {
        sortedEdges.push_back(edges.top());
        edges.pop();
    }
    std::sort(sortedEdges.begin(), sortedEdges.end(), [](const Edge& a, const Edge& b) {
        return a.weight < b.weight;
    });

    // Union-Find data structure initialization
    std::vector<int> parent(V), rank(V);
    for (size_t i = 0; i < V; ++i) {
        parent[i] = i;
        rank[i] = 0;
    }

    // Process sorted edges and add to MST using Union-Find
    for (const Edge& edge : sortedEdges) {
        size_t u = (size_t)edge.src;
        size_t v = (size_t)edge.dest;
        size_t set_u = (size_t)find(parent, u);
        size_t set_v = (size_t)find(parent, v);

        // Check for cycle (ignore if both vertices are already in MST)
        if (set_u != set_v) {
            mst.addEdge(u, v, edge.weight);
            Union(parent, rank, set_u, set_v);
        }
    }

    return mst;
}

// Helper function to find the set of an element (with path compression)
int TarjanMST::find(std::vector<int>& parent, int u) {
    if (parent[(size_t)u] != u) {
        parent[(size_t)u] = find(parent, parent[(size_t)u]);
    }
    return parent[(size_t)u];
}

// Helper function to union two sets (with union by rank)
void TarjanMST::Union(std::vector<int>& parent, std::vector<int>& rank, int u, int v) {
    size_t root_u = (size_t)find(parent, u);
    size_t root_v = (size_t)find(parent, v);

    if (root_u != root_v) {
        if (rank[root_u] > rank[root_v]) {
            parent[root_v] = root_u;
        } else if (rank[root_u] < rank[root_v]) {
            parent[root_u] = root_v;
        } else {
            parent[root_v] = root_u;
            rank[root_u]++;
        }
    }
}
