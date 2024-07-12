#include "Tree.hpp"
#include <stdexcept> // For std::out_of_range
#include <limits>    // For std::numeric_limits

Tree::Tree(int myVertices) : vertices(myVertices) {
    treeAdjList.resize(static_cast<size_t>(myVertices));
}

void Tree::addEdge(size_t u, size_t v) {
    treeAdjList[u].emplace_back(v, 0.0); // Default weight 0.0
    treeAdjList[v].emplace_back(u, 0.0); // Assuming an undirected tree
}

void Tree::printTree() const {
    for (size_t i = 0; i < treeAdjList.size(); ++i) {
        cout << i << " -> ";
        for (const auto& neighbor : treeAdjList[i]) {
            cout << "(" << neighbor.first << ", " << neighbor.second << ") ";
        }
        cout << endl;
    }
}

void Tree::addEdge(size_t u, size_t v, double weight) {
    treeAdjList[u].emplace_back(v, weight);
    treeAdjList[v].emplace_back(u, weight); // Since the tree is undirected
}

double Tree::calculateTotalWeight() const {
    double totalWeight = 0.0;
    for (const auto& neighbors : treeAdjList) {
        for (const auto& neighbor : neighbors) {
            totalWeight += neighbor.second; // Summing up the weights
        }
    }
    // Since each edge is counted twice, divide by 2
    return totalWeight / 2.0;
}

double Tree::calculateLongestDistance() const {
    // Implement logic to calculate the longest distance between two vertices
    // Placeholder logic: This should be replaced with actual implementation
    double maxDistance = 0.0;
    for (size_t i = 0; i < treeAdjList.size(); ++i) {
        for (const auto& neighbor : treeAdjList[i]) {
            // Assuming that each edge has weight, we need to calculate distances properly
            // Example distance logic, replace with actual
            if (neighbor.second > maxDistance) {
                maxDistance = neighbor.second;
            }
        }
    }
    return maxDistance;
}

double Tree::calculateAverageDistance() const {
    // Implement logic to calculate the average distance between all pairs of vertices
    // Placeholder logic: This should be replaced with actual implementation
    double totalDistance = 0.0;
    int edgeCount = 0;
    for (const auto& neighbors : treeAdjList) {
        for (const auto& neighbor : neighbors) {
            totalDistance += neighbor.second;
            ++edgeCount;
        }
    }
    // Since each edge is counted twice, divide by 2
    return edgeCount > 0 ? totalDistance / edgeCount : 0.0;
}

double Tree::calculateShortestDistance() const {
    // Implement logic to calculate the shortest distance between two vertices
    // Placeholder logic: This should be replaced with actual implementation
    double minDistance = std::numeric_limits<double>::max();
    for (const auto& neighbors : treeAdjList) {
        for (const auto& neighbor : neighbors) {
            if (neighbor.second < minDistance) {
                minDistance = neighbor.second;
            }
        }
    }
    return minDistance;
}

double Tree::getEdgeWeight(size_t u, size_t v) const {
    for (const auto& neighbor : treeAdjList[u]) {
        if (neighbor.first == v) {
            return neighbor.second; // Return weight if edge found
        }
    }
    throw std::out_of_range("Edge not found"); // Throw exception if edge not found
}

int Tree::getEdgesCount() const {
    int count = 0;
    for (const auto& neighbors : treeAdjList) {
        count += (int)neighbors.size();
    }
    // Each edge is counted twice, so divide by 2
    return count / 2;
}