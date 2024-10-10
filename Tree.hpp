#ifndef TREE_HPP
#define TREE_HPP

#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <future>
#include <iostream>
#include <stdexcept>
#include <limits>
#include <functional>

// Leader-Follower class definition
class LeaderFollower {
public:
    using Task = std::function<void()>;

    LeaderFollower(size_t threadCount);
    ~LeaderFollower();

    // Submits a task to the thread pool
    std::future<void> submitTask(Task task);

private:
    void workerThread();  // Each worker thread will execute this function

    std::queue<Task> tasks;
    std::mutex tasksMutex;
    std::condition_variable tasksCondition;
    bool stop = false;
    std::vector<std::thread> workers;
};

// Tree class definition
class Tree {
public:
    Tree(int myVertices);
    // Constructor to initialize the Tree with n vertices
    Tree(int n) : treeAdjList(n) {}
    Tree();

    void addEdge(size_t u, size_t v, double weight);
    void addEdge(size_t u, size_t v);
    bool isValid() const;
    void printTree() const;
    void printTree(std::ostream& os) const;

    // Metric functions
    double calculateTotalWeight() const;
    double calculateLongestDistance() const;
    double calculateAverageDistance() const;
    double calculateShortestDistance() const;

    double getEdgeWeight(size_t u, size_t v) const;
    int getEdgesCount() const;
    std::vector<std::vector<std::pair<size_t, double>>> treeAdjList;
    int vertices;  
};

#endif // TREE_HPP
