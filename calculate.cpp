#include "calculate.hpp"
using namespace std;

#include <unistd.h>  // For socket communication
#include <string>
#include <limits>

// 1. Calculate the total weight of the MST and send it to the client through the socket
void calculateTotalWeight(const Tree& tree, int clientSocket) {
    double totalWeight = 0.0;

    for (const auto& neighbors : tree.treeAdjList) {
        for (const auto& neighbor : neighbors) {
            totalWeight += neighbor.second;  // Add the weight of each edge
        }
    }
    totalWeight /= 2.0;  // Each edge is counted twice (u->v and v->u)

}

// 2. Calculate the longest distance between two vertices and send it to the client through the socket
void calculateLongestDistance(const Tree& tree, int clientSocket) {
    double maxDistance = 0.0;

    for (const auto& neighbors : tree.treeAdjList) {
        for (const auto& neighbor : neighbors) {
            if (neighbor.second > maxDistance) {
                maxDistance = neighbor.second;  // Track the maximum edge weight
            }
        }
    }

}

// 3. Calculate the average distance between any two vertices and send it to the client through the socket
void calculateAverageDistance(const Tree& tree, int clientSocket) {
    double totalDistance = 0.0;
    int pairCount = 0;

    // Loop through the adjacency list to accumulate distances
    for (size_t i = 0; i < tree.treeAdjList.size(); ++i) {
        for (const auto& neighbor : tree.treeAdjList[i]) {
            if (i < neighbor.first) {  // Ensure that each edge is only counted once
                totalDistance += neighbor.second;
                pairCount++;
            }
        }
    }

    double averageDistance = pairCount > 0 ? (totalDistance / pairCount) : 0.0;

}
