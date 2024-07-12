#include "Graph.hpp"
#include "Tree.hpp"
#include "MSTFactory.hpp"

int main() {
    Graph graph(5);

    // Add edges to the graph with weights
    graph.addEdge(0, 1, 2.0);
    graph.addEdge(0, 2, 4.0);
    graph.addEdge(1, 2, 1.5);
    graph.addEdge(1, 3, 3.0);
    graph.addEdge(2, 4, 5.0);

    // Print the graph
    graph.printGraph();

    // Create MST using Kruskal's algorithm
    auto mstStrategy = MSTFactory::createMSTStrategy(MSTFactory::Algorithm::KRUSKAL);
    Tree mst = mstStrategy->computeMST(graph);

    // Print the MST
    mst.printTree();

    // Calculate additional data
    double totalWeight = mst.calculateTotalWeight();
    double longestDistance = mst.calculateLongestDistance();
    double averageDistance = mst.calculateAverageDistance();
    double shortestDistance = mst.calculateShortestDistance();

    cout << "Total Weight: " << totalWeight << endl;
    cout << "Longest Distance: " << longestDistance << endl;
    cout << "Average Distance: " << averageDistance << endl;
    cout << "Shortest Distance: " << shortestDistance << endl;

    return 0;
}
