#include "Tree.hpp"
#include <utility>  // for std::move

// LeaderFollower class implementation
LeaderFollower::LeaderFollower(size_t threadCount) {
    for (size_t i = 0; i < threadCount; ++i) {
        workers.emplace_back(&LeaderFollower::workerThread, this);
    }
}

LeaderFollower::~LeaderFollower() {
    {
        std::unique_lock<std::mutex> lock(tasksMutex);
        stop = true;
    }
    tasksCondition.notify_all();
    for (auto& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

std::future<void> LeaderFollower::submitTask(Task task) {
    // Wrap the task in a std::packaged_task
    auto packagedTask = std::make_shared<std::packaged_task<void()>>(std::move(task));
    std::future<void> result = packagedTask->get_future();

    {
        // Lock the task queue before pushing
        std::unique_lock<std::mutex> lock(tasksMutex);

        // Push the task into the queue
        tasks.push([packagedTask]() {
            (*packagedTask)();  // Dereference and call the packaged task
        });
    }

    // Notify one waiting thread that a new task is available
    tasksCondition.notify_one();

    return result;
}


void LeaderFollower::workerThread() {
    while (true) {
        Task task;
        {
            std::unique_lock<std::mutex> lock(tasksMutex);
            tasksCondition.wait(lock, [this] { return stop || !tasks.empty(); });
            if (stop && tasks.empty()) {
                return;
            }
            task = std::move(tasks.front());
            tasks.pop();
        }
        task();  // Execute the task
    }
}

// Tree class implementation
Tree::Tree(int myVertices) : vertices(myVertices) {
    treeAdjList.resize(static_cast<size_t>(myVertices));
}

Tree::Tree() {
    treeAdjList.resize(static_cast<size_t>(0));
}

bool Tree::isValid() const {
    return !treeAdjList.empty();
}

void Tree::addEdge(size_t u, size_t v) {
    treeAdjList[u].emplace_back(v, 0.0); // Default weight 0.0
    treeAdjList[v].emplace_back(u, 0.0); // Assuming an undirected tree
}

void Tree::printTree() const {
    for (size_t i = 0; i < treeAdjList.size(); ++i) {
        std::cout << i << " -> ";
        for (const auto& neighbor : treeAdjList[i]) {
            std::cout << "(" << neighbor.first << ", " << neighbor.second << ") ";
        }
        std::cout << std::endl;
    }
}

void Tree::printTree(std::ostream& os) const {
    for (size_t i = 0; i < treeAdjList.size(); ++i) {
        os << i << " -> ";
        for (const auto& neighbor : treeAdjList[i]) {
            os << "(" << neighbor.first << ", " << neighbor.second << ") ";
        }
        os << std::endl;
    }
}

void Tree::addEdge(size_t u, size_t v, double weight) {
    treeAdjList[u].emplace_back(v, weight);
    treeAdjList[v].emplace_back(u, weight); // Since the tree is undirected
}

double Tree::calculateTotalWeight() const {
    LeaderFollower lf(4);  // Create LF with 4 worker threads

    double totalWeight = 0.0;
    std::mutex weightMutex;

    std::vector<std::future<void>> futures;
    for (const auto& neighbors : treeAdjList) {
        futures.push_back(lf.submitTask([&]() {
            double localWeight = 0.0;
            for (const auto& neighbor : neighbors) {
                localWeight += neighbor.second;
            }
            std::lock_guard<std::mutex> guard(weightMutex);
            totalWeight += localWeight;
        }));
    }

    for (auto& future : futures) {
        future.get();
    }

    return totalWeight / 2.0;
}

double Tree::calculateLongestDistance() const {
    LeaderFollower lf(4);

    double maxDistance = 0.0;
    std::mutex maxMutex;

    std::vector<std::future<void>> futures;
    for (const auto& neighbors : treeAdjList) {
        futures.push_back(lf.submitTask([&]() {
            double localMax = 0.0;
            for (const auto& neighbor : neighbors) {
                if (neighbor.second > localMax) {
                    localMax = neighbor.second;
                }
            }
            std::lock_guard<std::mutex> guard(maxMutex);
            if (localMax > maxDistance) {
                maxDistance = localMax;
            }
        }));
    }

    for (auto& future : futures) {
        future.get();
    }

    return maxDistance;
}

double Tree::calculateAverageDistance() const {
    LeaderFollower lf(4);

    double totalDistance = 0.0;
    int edgeCount = 0;
    std::mutex distMutex;

    std::vector<std::future<void>> futures;
    for (const auto& neighbors : treeAdjList) {
        futures.push_back(lf.submitTask([&]() {
            double localDistance = 0.0;
            int localEdges = 0;
            for (const auto& neighbor : neighbors) {
                localDistance += neighbor.second;
                ++localEdges;
            }
            std::lock_guard<std::mutex> guard(distMutex);
            totalDistance += localDistance;
            edgeCount += localEdges;
        }));
    }

    for (auto& future : futures) {
        future.get();
    }

    return edgeCount > 0 ? totalDistance / edgeCount : 0.0;
}

double Tree::calculateShortestDistance() const {
    LeaderFollower lf(4);

    double minDistance = std::numeric_limits<double>::max();
    std::mutex minMutex;

    std::vector<std::future<void>> futures;
    for (const auto& neighbors : treeAdjList) {
        futures.push_back(lf.submitTask([&]() {
            double localMin = std::numeric_limits<double>::max();
            for (const auto& neighbor : neighbors) {
                if (neighbor.second < localMin) {
                    localMin = neighbor.second;
                }
            }
            std::lock_guard<std::mutex> guard(minMutex);
            if (localMin < minDistance) {
                minDistance = localMin;
            }
        }));
    }

    for (auto& future : futures) {
        future.get();
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