#ifndef TREE_HPP
#define TREE_HPP

#include <vector>
#include <iostream>

using namespace std;

class Tree {
public:
    Tree(int myVertices);
    Tree();
    bool isValid() const;
    void addEdge(size_t u, size_t v);
    void printTree() const;
    void printTree(std::ostream& os) const;
    void addEdge(size_t u, size_t v, double weight);
    double calculateTotalWeight() const;
    double calculateLongestDistance() const;
    double calculateAverageDistance() const;
    double calculateShortestDistance() const;
    double getEdgeWeight(size_t u, size_t v) const; // Declaration of getEdgeWeight method 
    int getEdgesCount() const; // Add a method to get the number of edges

private:
    int vertices;
    vector<vector<pair<size_t, double>>> treeAdjList; // Updated to store pair<size_t, double>
};

#endif // TREE_HPP
