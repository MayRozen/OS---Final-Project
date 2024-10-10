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

    string msg = "The Total weight of the MST is: " + to_string(totalWeight) + "\n";
    write(clientSocket, msg.c_str(), msg.size());

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
    string msg = "Longest distance between two vertices is: " + to_string(maxDistance) + "\n";
    write(clientSocket, msg.c_str(), msg.size());

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

    // Calculate the average distance
    double average = pairCount > 0 ? totalDistance / pairCount : 0.0;
    string msg = "The average distance between vertecies in the graph is: " + to_string(average) + "\n";
    write(clientSocket, msg.c_str(), msg.size());
}

void calculateShortestDistance(const Tree& tree, int clientSocket) {
    // Initialize minDistance to the largest possible value
    double minDistance = std::numeric_limits<double>::max();

    for (const auto& neighbors : tree.treeAdjList) {
        for (const auto& neighbor : neighbors) {
            if (neighbor.second < minDistance && neighbor.second > 0) {  
                // Track the minimum edge weight
                minDistance = neighbor.second;
            }
        }
    }
     string msg = "Shortest distance between two vertices is: " + to_string(minDistance) + "\n";
    write(clientSocket, msg.c_str(), msg.size());
}
