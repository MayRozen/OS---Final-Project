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
#include <sstream>
#include <functional>
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

// Shared resources for communication between stages
std::queue<int> requestQueue;
std::queue<string> responseQueue;
std::mutex mtx;
std::condition_variable request_cv;
std::condition_variable response_cv;
bool running = true; // Flag to indicate server status

// Stage 1: Receive Request
/* accepts incoming client connections and pushes the client file descriptors into the requestQueue.
 This stage is responsible for receiving requests from clients.
*/
void receiveRequests() {
    /*accepts new client connections and pushes the client file descriptor (new_fd) into the 
    requestQueue. This represents a new connection from a client, and this descriptor is 
    what will be processed later.
    */
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    char s[INET6_ADDRSTRLEN];

    while (running) {
        sin_size = sizeof their_addr;
        int new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
        cout << "server: got connection from " << s << endl;

        {
            std::lock_guard<std::mutex> lock(mtx);
            requestQueue.push(new_fd);
        }
        request_cv.notify_one(); // Notify the processing stage
    }
}

// Stage 2: Process Commands
/* waits for requests to be available in requestQueue, pops a request, and processes it. 
It reads commands from the client, performs various actions (MST computation), and sends the response back to the client.
*/
void processCommands() {
    /*When a client file descriptor is available, it pops the client_fd from 
    the queue and uses it to communicate with the client 
    */
    while (running) {
        int client_fd;

        {
            std::unique_lock<std::mutex> lock(mtx);
            request_cv.wait(lock, [] { return !requestQueue.empty() || !running; });

            if (!running && requestQueue.empty()) {
                break; // Exit if server is shutting down and queue is empty
            }

            client_fd = requestQueue.front();
            requestQueue.pop();
        }

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
                close(client_fd);
                return;
            }

            std::istringstream iss(command);
            std::string action;
            iss >> action;
            std::string response;

            // Create a new graph
            if (action == "new_graph") {
                int num_vertices;
                iss >> num_vertices;
                graph = Graph(num_vertices); // Create a new graph with the specified number of vertices
                response = "New graph created with " + std::to_string(num_vertices) + " vertices.\n";
            }
            // Add edge: format "add_edge vertex1 vertex2 weight"
            else if (action == "add_edge") {
                size_t v1, v2;
                double weight;
                iss >> v1 >> v2 >> weight;
                graph.addEdge(v1, v2, weight);
                response = "Edge added between " + to_string(v1) + " and " + to_string(v2) + " with weight " + std::to_string(weight) + ".\n";
            }
            // Remove edge: format "remove_edge vertex1 vertex2"
            else if (action == "remove_edge") {
                size_t v1, v2;
                iss >> v1 >> v2;
                graph.removeEdge(v1, v2);
                response = "Edge removed between " + std::to_string(v1) + " and " + std::to_string(v2) + ".\n";
            }
            // Build MST using specified algorithm and return tree
            else if (action == "MST") {
                std::string algorithm;
                iss >> algorithm;

                if (algorithm == "Kruskal") {
                    auto mstStrategy = MSTFactory::createMSTStrategy(MSTFactory::Algorithm::KRUSKAL);
                    mst = mstStrategy->computeMST(graph);
                } else if (algorithm == "Prim") {
                    auto mstStrategy = MSTFactory::createMSTStrategy(MSTFactory::Algorithm::PRIM);
                    mst = mstStrategy->computeMST(graph);
                } else if (algorithm == "Boruvka") {
                    auto mstStrategy = MSTFactory::createMSTStrategy(MSTFactory::Algorithm::Boruvka);
                    mst = mstStrategy->computeMST(graph);
                } else if (algorithm == "Tarjan") {
                    auto mstStrategy = MSTFactory::createMSTStrategy(MSTFactory::Algorithm::Tarjan);
                    mst = mstStrategy->computeMST(graph);
                } else if (algorithm == "Integer") {
                    auto mstStrategy = MSTFactory::createMSTStrategy(MSTFactory::Algorithm::Integer);
                    mst = mstStrategy->computeMST(graph);
                } else {
                    response = "Unknown MST algorithm\n";
                    send(client_fd, response.c_str(), response.length(), 0);
                    continue;
                }

                // Send back the MST result to the client
                std::ostringstream oss;
                oss << "MST Computed using " << algorithm << ":" << std::endl;
                mst.printTree(oss);  // Assuming printTree can accept an ostream
                response = oss.str();
            }
            else if (action == "calculate_mst_data") {
                if (!mst.isValid()) {
                    response = "MST not computed yet. Please compute MST first.\n";
                } else {
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
                    response = oss.str();
                }
            }
            // Print the current graph
            else if (action == "print_graph") {
                std::ostringstream oss;
                graph.printGraph(oss);  // Assuming printGraph can accept an ostream
                response = oss.str();
            }
            else {
                response = "Unknown command\n";
            }

            // Send the response to the client
            send(client_fd, response.c_str(), response.length(), 0);
        }
        close(client_fd);
        std::cout << "Request handled." << std::endl;
    }
}

// Stage 3: Send Response (Not implemented in this case as responses are sent in processing stage)

// Main function
int main() {
    struct addrinfo hints, *servinfo, *p;
    int yes = 1;
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

    // Start pipeline threads
    std::thread receiverThread(receiveRequests);
    std::thread processorThread(processCommands);

    receiverThread.join();
    processorThread.join();

    close(sockfd);  // Ensure socket is closed before exit
    return 0;
}
