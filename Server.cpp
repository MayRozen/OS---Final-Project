#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <csignal>
#include <sys/wait.h>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>
#include <boost/asio.hpp>
#include "Graph.hpp"
#include "Tree.hpp"
#include "MSTFactory.hpp"

#define PORT "9034"  // Port to listen on
#define BACKLOG 10   // Number of pending connections queue will hold

using namespace std;

int sockfd; // Global socket descriptor for cleanup

// Function to extract address information from sockaddr structure
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// Signal handler to properly close the socket on SIGINT or SIGTERM
void signalHandler(int signum) {
    if (sockfd != -1) {
        close(sockfd);
    }
    std::cout << "\nServer shutting down gracefully...\n";
    exit(signum);
}

// Leader-Follower pattern class to manage thread pool
class LeaderFollower {
    std::vector<std::thread> threads;
    std::mutex mtx;
    boost::asio::io_context io_context;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard;

public:
    LeaderFollower(int n_threads)
        : work_guard(boost::asio::make_work_guard(io_context)) {
        for (int i = 0; i < n_threads; ++i) {
            threads.emplace_back([this, i]() { this->threadLoop(i); });
        }
    }

    ~LeaderFollower() {
        io_context.stop();
        for (auto& thread : threads) {
            thread.join();
        }
    }

    void submit(std::function<void()> task) {
        boost::asio::post(io_context, task);
    }

private:
    void threadLoop(int id) {
        std::cout << "Thread " << id << " is running" << std::endl;
        io_context.run();
    }
};

// Function to handle client requests and execute the binary
void handleRequest(int client_fd) {
    std::cout << "Handling request..." << std::endl;

    char buffer[1024];
    std::string command;
    Graph graph(5); // Default graph with 5 vertices
    Tree mst;
    
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);  // Receive command from client
        if (bytes_received <= 0) {
            break;  // Exit loop if the connection is closed or there's an error
        }

        command = std::string(buffer);
        command = command.substr(0, command.find("\n")); // Remove trailing newline character
        std::cout << "Received command: " << command << std::endl;

        if (command == "end") {
            break;
        }
        
        std::istringstream iss(command);
        std::string action;
        iss >> action;

        // Create a new graph
        if (action == "new_graph") {
            int num_vertices;
            iss >> num_vertices;
            graph = Graph(num_vertices); // Create a new graph with the specified number of vertices
            std::string response = "New graph created with " + std::to_string(num_vertices) + " vertices.\n";
            send(client_fd, response.c_str(), response.length(), 0);
        }
        // Add edge: format "add_edge vertex1 vertex2 weight"
        else if (action == "add_edge") {
            size_t v1, v2;
            double weight;
            iss >> v1 >> v2 >> weight;
            graph.addEdge(v1, v2, weight);
            std::string response = "Edge added between " + to_string(v1) + " and " + to_string(v2) + " with weight " + std::to_string(weight) + ".\n";
            send(client_fd, response.c_str(), response.length(), 0);
        }
        // Remove edge: format "remove_edge vertex1 vertex2"
        else if (action == "remove_edge") {
            size_t v1, v2;
            iss >> v1 >> v2;
            graph.removeEdge(v1, v2);
            std::string response = "Edge removed between " + std::to_string(v1) + " and " + std::to_string(v2) + ".\n";
            send(client_fd, response.c_str(), response.length(), 0);
        }
        // Build MST using specified algorithm and return tree
        else if (action == "MST") {
            std::string algorithm;
            iss >> algorithm;
            
            if (algorithm == "Kruskal") {
                auto mstStrategy = MSTFactory::createMSTStrategy(MSTFactory::Algorithm::KRUSKAL);
                mst = mstStrategy->computeMST(graph);
            } 
            else if (algorithm == "Prim") {
                auto mstStrategy = MSTFactory::createMSTStrategy(MSTFactory::Algorithm::PRIM);
                mst = mstStrategy->computeMST(graph);
            } 
            else if (algorithm == "Boruvka") {
                auto mstStrategy = MSTFactory::createMSTStrategy(MSTFactory::Algorithm::Boruvka);
                mst = mstStrategy->computeMST(graph);
            } 
            else if (algorithm == "Tarjan") {
                auto mstStrategy = MSTFactory::createMSTStrategy(MSTFactory::Algorithm::Tarjan);
                mst = mstStrategy->computeMST(graph);
            }
            else if (algorithm == "Integer") {
                auto mstStrategy = MSTFactory::createMSTStrategy(MSTFactory::Algorithm::Integer);
                mst = mstStrategy->computeMST(graph);
            }
            else {
                const char *error_msg = "Unknown MST algorithm\n";
                send(client_fd, error_msg, strlen(error_msg), 0);
                continue;
            }

            // Send back the MST result to the client
            std::ostringstream oss;
            oss << "MST Computed using " << algorithm << ":" << std::endl;
            mst.printTree(oss);  // Assuming printTree can accept an ostream
            std::string result = oss.str();
            send(client_fd, result.c_str(), result.length(), 0);
            

        }
        else if (action == "calculate_mst_data") {
            if (!mst.isValid()) {
                const char *error_msg = "MST not computed yet. Please compute MST first.\n";
                send(client_fd, error_msg, strlen(error_msg), 0);
                continue;
            }

            // Perform the data calculations
            double totalWeight = mst.calculateTotalWeight();
            double longestDistance = mst.calculateLongestDistance();
            double averageDistance = mst.calculateAverageDistance();
            double shortestDistance = mst.calculateShortestDistance();

            // Prepare the response
            std::ostringstream oss;
            oss << "MST Data:\n";
            oss << "Total Weight: " << totalWeight << "\n";
            oss << "Longest Distance: " << longestDistance << "\n";
            oss << "Average Distance: " << averageDistance << "\n";
            oss << "Shortest Distance: " << shortestDistance << "\n";
            std::string result = oss.str();

            // Send the response to the client
            send(client_fd, result.c_str(), result.length(), 0);
        }

        // Print the current graph
        else if (action == "print_graph") {
            std::ostringstream oss;
            graph.printGraph(oss);  // Assuming printGraph can accept an ostream
            std::string result = oss.str();
            send(client_fd, result.c_str(), result.length(), 0);
        }
        else {
            const char *error_msg = "Unknown command\n";
            send(client_fd, error_msg, strlen(error_msg), 0);
        }
    }

    close(client_fd);
    std::cout << "Request handled." << std::endl;
}


int main() {
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    // Register signal handlers to clean up on SIGINT and SIGTERM
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;  // Use AF_INET for IPv4
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
            close(sockfd);
            freeaddrinfo(servinfo);
            return 1;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            perror("server: bind");
            close(sockfd);
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
        close(sockfd);
        return 1;
    }

    cout << "server: waiting for connections..." << endl;

    LeaderFollower leaderFollower(4); // Create a thread pool with 4 threads

    while (true) {
        sin_size = sizeof their_addr;
        int new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
        cout << "server: got connection from " << s << endl;

        leaderFollower.submit([new_fd]() {
            handleRequest(new_fd);
        });
    }

    close(sockfd);  // Ensure socket is closed before exit
    return 0;
}

//**************************** how to run the code **********************************//


// nc 127.0.0.1 9034
// new_graph 5
// "New graph created with 5 vertices."

// add_edge 0 1 2.0
// "Edge added between 0 and 1 with weight 2.0."

// add_edge 0 2 3.0
// "Edge added between 0 and 2 with weight 3.0."

// print_graph
// "Current graph:
// 0 -> (1, 2) (2, 3) 
// 1 -> (0, 2) 
// 2 -> (0, 3) 
// 3 -> 
// 4 -> "

// MST Kruskal
// "MST Computed using Kruskal:
// 0 -> (1, 2) (2, 3) 
// 1 -> (0, 2) 
// 2 -> (0, 3) 
// 3 -> 
// 4 -> "
// Edge 0-1 with weight 2.0
// Edge 0-2 with weight 3.0

// calculate_mst_data

// remove_edge 1 2
// "Edge removed between 1 and 2."

// add_edge 2 3 6.0
// "Edge added between 2 and 3 with weight 6.000000."

// print_graph
// "0 -> (1, 2) (2, 3) 
// 1 -> (0, 2) 
// 2 -> (0, 3) (3, 6) 
// 3 -> (2, 6) 
// 4 -> "

// end

