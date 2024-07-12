#include "Graph.hpp"

Graph::Graph(int myVertices) : vertices(myVertices) {
    adjList.resize((size_t)myVertices);
}

void Graph::addEdge(size_t u, size_t v, double weight) {
    adjList[u].emplace_back(v, weight);
    adjList[v].emplace_back(u, weight); // Since the graph is undirected
}

bool Graph::isConnected() {
    vector<bool> visited((size_t)vertices, false);
    size_t i;

    // Find a vertex with a non-zero degree
    for (i = 0; i < (size_t)vertices; i++) {
        if (!adjList[i].empty()) {
            break;
        }
    }

    // If no edges in the graph, return true
    if (i == (size_t)vertices) {
        return true;
    }

    // Start DFS traversal from a vertex with a non-zero degree
    DFS(i, visited);

    // Check if all vertices with non-zero degree are visited
    for (i = 0; i < (size_t)vertices; i++) {
        if (!adjList[i].empty() && !visited[i]) {
            return false;
        }
    }

    return true;
}

void Graph::DFS(size_t v, vector<bool>& visited) {
    visited[v] = true;

    for (auto& neighbor : adjList[v]) {
        if (!visited[(size_t)neighbor.first]) {
            DFS((size_t)neighbor.first, visited);
        }
    }
}

int Graph::findStartVertex() {
    for (size_t i = 0; i < (size_t)vertices; i++) {
        if (adjList[i].size() % 2 != 0) {
            return i;
        }
    }
    return 0;
}

bool Graph::isEulerian() {
    if (!isConnected()) {
        return false;
    }

    int odd = 0;
    for (size_t i = 0; i < (size_t)vertices; i++) {
        if (adjList[i].size() % 2 != 0) {
            odd++;
        }
    }

    return (odd == 0);
}

void Graph::findEulerCircuit() {
    if (!isEulerian()) {
        cout << "The graph does not have an Eulerian circuit." << endl;
        return;
    }

    list<int> circuit;
    map<pair<int, int>, bool> edgeVisited;
    vector<int> stack;
    int startVertex = findStartVertex();

    stack.push_back(startVertex);

    while (!stack.empty()) {
        size_t v = (size_t)stack.back();

        bool found = false;
        for (auto& neighbor : adjList[v]) {
            if (!edgeVisited[{v, neighbor.first}]) {
                edgeVisited[{v, neighbor.first}] = edgeVisited[{neighbor.first, v}] = true;
                stack.push_back(neighbor.first);
                found = true;
                break;
            }
        }

        if (!found) {
            circuit.push_front(v);
            stack.pop_back();
        }
    }

    for (int vertex : circuit) {
        cout << vertex << " ";
    }
    cout << endl;
}

void Graph::printGraph() {
    for (size_t i = 0; i < (size_t)vertices; i++) {
        cout << i << " -> ";
        for (auto& neighbor : adjList[i]) {
            cout << "(" << neighbor.first << ", " << neighbor.second << ") ";
        }
        cout << endl;
    }
}

int Graph::getVertices() const {
    return vertices;
}

const list<pair<int, double>>& Graph::getAdjList(size_t u) const {
    return adjList[u];
}

double Graph::getWeight(size_t u, size_t v) const {
    for (const auto& neighbor : adjList[u]) {
        if ((size_t)neighbor.first == v) {
            return neighbor.second;
        }
    }
    return -1; // Or some other error value
}
