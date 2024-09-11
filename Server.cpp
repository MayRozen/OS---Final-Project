#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <csignal>
#include <sys/wait.h>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>
#include <unordered_map>
#include <boost/asio.hpp>

#define PORT "9034"  // Port to listen on
#define BACKLOG 10   // Number of pending connections queue will hold

using namespace std;

// Function to extract address information from sockaddr structure
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// Signal handler to reap dead processes
void sigchld_handler([[maybe_unused]] int s) {
    while(waitpid(-1, nullptr, WNOHANG) > 0);
}

// Graph class to manage nodes and edges
class Graph {
    unordered_map<int, vector<pair<int, double>>> adjList; // Node -> [(neighbor, weight)]
    
public:
    // Add edge to the graph
    void addEdge(int u, int v, double weight) {
        adjList[u].emplace_back(v, weight);
        adjList[v].emplace_back(u, weight); // Assuming undirected graph
    }

    // Display the graph (for debugging)
    void displayGraph() {
        for (const auto& node : adjList) {
            cout << node.first << " -> ";
            for (const auto& neighbor : node.second) {
                cout << "(" << neighbor.first << ", " << neighbor.second << ") ";
            }
            cout << endl;
        }
    }

    // Prim's algorithm to compute the MST
    double computeMST() {
        if (adjList.empty()) return 0;

        unordered_map<int, bool> inMST;
        priority_queue<pair<double, int>, vector<pair<double, int>>, greater<>> pq; // Min-heap for edge weights
        int startNode = adjList.begin()->first;
        pq.emplace(0, startNode); // Start from an arbitrary node

        double totalCost = 0;
        while (!pq.empty()) {
            auto [cost, node] = pq.top(); pq.pop();
            if (inMST[node]) continue; // Skip if already in MST

            inMST[node] = true;
            totalCost += cost;

            for (auto& [neighbor, weight] : adjList[node]) {
                if (!inMST[neighbor]) {
                    pq.emplace(weight, neighbor);
                }
            }
        }

        return totalCost;
    }
};

// ActiveObject class to process tasks asynchronously
class ActiveObject {
    std::queue<std::function<void()>> tasks;
    std::mutex mtx;
    std::condition_variable cv;
    std::thread worker;
    bool stop;

public:
    ActiveObject() : stop(false) {
        worker = std::thread([this]() { this->process(); });
    }

    ~ActiveObject() {
        {
            std::lock_guard<std::mutex> lock(mtx);
            stop = true;
        }
        cv.notify_all();
        worker.join();
    }

    void submit(std::function<void()> task) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            tasks.push(task);
        }
        cv.notify_one();
    }

private:
    void process() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [this] { return !tasks.empty() || stop; });

                if (stop && tasks.empty()) return;
                task = tasks.front();
                tasks.pop();
            }
            task();
        }
    }
};

// Leader-Follower pattern class to manage thread pool
class LeaderFollower {
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks; // Add this member
    std::mutex mtx;
    std::condition_variable cv;
    bool stop;
    boost::asio::io_context io_context;

public:
    LeaderFollower(int n_threads) : stop(false) {
        for (int i = 0; i < n_threads; ++i) {
            threads.emplace_back([this, i]() { this->threadLoop(i); });
        }
    }

    ~LeaderFollower() {
        {
            std::lock_guard<std::mutex> lock(mtx);
            stop = true;
        }
        cv.notify_all();
        for (auto& thread : threads) {
            thread.join();
        }
    }

    void submit(std::function<void()> task) {
        boost::asio::post(io_context, task);
    }

private:
    void threadLoop(int id) {
        while (true) {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] { return stop || !tasks.empty(); });

            if (stop && tasks.empty()) return;

            std::cout << "Thread " << id << " is the leader" << std::endl;

            while (!tasks.empty()) {
                auto task = tasks.front();
                tasks.pop();
                lock.unlock();
                task();
                lock.lock();
            }
        }
    }
};

// Function to handle the MST problem-solving and respond to the client
void handleRequest(int client_fd, Graph &graph) {
    char buffer[256];
    int n = read(client_fd, buffer, sizeof(buffer) - 1);

    if (n < 0) {
        perror("Error reading from socket");
        close(client_fd);
        return;
    }
    if (n == 0) {
        // Connection closed by client
        close(client_fd);
        return;
    }

    buffer[n] = '\0'; // Null-terminate the buffer

    std::string request(buffer);
    std::istringstream iss(request);
    std::string command;
    iss >> command;

    if (command == "ADD_EDGE") {
        int u, v;
        double weight;
        if (iss >> u >> v >> weight) {
            graph.addEdge(u, v, weight);
            const char *response = "Edge added successfully\n";
            send(client_fd, response, strlen(response), 0);
            // Display the current state of the graph
            graph.displayGraph();
        } else {
            const char *response = "Invalid ADD_EDGE command format\n";
            send(client_fd, response, strlen(response), 0);
        } 
    } else if (command == "COMPUTE_MST") {
        double mstCost = graph.computeMST();
        std::ostringstream oss;
        oss << "MST total cost: " << mstCost << "\n";
        send(client_fd, oss.str().c_str(), oss.str().length(), 0);
        // Display the current state of the graph
        graph.displayGraph();
    } else {
        const char *response = "Unknown command\n";
        send(client_fd, response, strlen(response), 0);
    }

    close(client_fd);
    std::cout << "Request handled." << std::endl;
}

int main() {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    struct sigaction sa;

    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        cerr << "getaddrinfo: " << gai_strerror(rv) << endl;
        return 1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo);

    if (p == NULL) {
        cerr << "server: failed to bind" << endl;
        return 2;
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    cout << "server: waiting for connections..." << endl;

    LeaderFollower leaderFollower(4); // Create a thread pool with 4 threads
    Graph graph; // Create a graph instance

    while (true) {
        sin_size = sizeof their_addr;
        int new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
        cout << "server: got connection from " << s << endl;

        leaderFollower.submit([new_fd, &graph]() {
            handleRequest(new_fd, graph);
        });
    }

    return 0;
}
