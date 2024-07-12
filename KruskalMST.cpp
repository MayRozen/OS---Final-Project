#include "KruskalMST.hpp"
#include <algorithm>
#include <vector>
#include <numeric>

using namespace std;

struct Edge {
    int src, dest;
    double weight;
};

// A structure to represent a subset for union-find
struct Subset {
    int parent;
    int rank;
};

// A utility function to find set of an element i (uses path compression technique)
int find(Subset subsets[], int i) {
    // find root and make root as parent of i (path compression)
    if (subsets[i].parent != i) {
        subsets[i].parent = find(subsets, subsets[i].parent);
    }
    return subsets[i].parent;
}

// A function that does union of two sets of x and y (uses union by rank)
void Union(Subset subsets[], int x, int y) {
    int xroot = find(subsets, x);
    int yroot = find(subsets, y);

    // Attach smaller rank tree under root of high rank tree (Union by Rank)
    if (subsets[xroot].rank < subsets[yroot].rank) {
        subsets[xroot].parent = yroot;
    } else if (subsets[xroot].rank > subsets[yroot].rank) {
        subsets[yroot].parent = xroot;
    } else {
        subsets[yroot].parent = xroot;
        subsets[xroot].rank++;
    }
}

Tree KruskalMST::computeMST(const Graph& graph) {
    int V = graph.getVertices();
    vector<Edge> edges;

    // Convert adjacency list to edge list
    for (size_t u = 0; u < static_cast<size_t>(V); ++u) {
        for (const auto& neighbor : graph.getAdjList(static_cast<size_t>(u))) {
            if (static_cast<size_t>(u) < (size_t)neighbor.first) {
                edges.push_back({static_cast<int>(u), neighbor.first, neighbor.second});
            }
        }
    }

    // Sort edges in increasing order on basis of cost
    sort(edges.begin(), edges.end(), [](Edge a, Edge b) {
        return a.weight < b.weight;
    });

    // Allocate memory for creating V subsets
    Subset* subsets = new Subset[V];
    for (int v = 0; v < V; ++v) {
        subsets[v].parent = v;
        subsets[v].rank = 0;
    }

    Tree mst(V);

    for (const Edge& edge : edges) {
        int x = find(subsets, edge.src);
        int y = find(subsets, edge.dest);

        if (x != y) {
            mst.addEdge((size_t)edge.src, (size_t)edge.dest, edge.weight);
            Union(subsets, x, y);
        }
    }

    delete[] subsets;  // Clean up allocated memory if needed

    return mst;  // Ensure to return a Tree object
}
