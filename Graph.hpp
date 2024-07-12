#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <vector>
#include <list>
#include <map>
#include <iostream>
#include <utility>

using namespace std;

class Graph {
public:
    Graph(int myVertices);
    void addEdge(size_t u, size_t v, double weight);
    bool isConnected();
    void DFS(size_t v, vector<bool>& visited);
    int findStartVertex();
    bool isEulerian();
    void findEulerCircuit();
    void printGraph();
    int getVertices() const;
    const list<pair<int, double>>& getAdjList(size_t u) const;
    double getWeight(size_t u, size_t v) const;

private:
    int vertices;
    vector<list<pair<int, double>>> adjList;
};

#endif // GRAPH_HPP
