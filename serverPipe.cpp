#include <iostream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <csignal>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include "Graph.hpp"
#include "Tree.hpp"
#include "MSTFactory.hpp"
#include "calculate.hpp"

#define PORT "9034"  // Port to listen on
#define BACKLOG 10   // Number of pending connections queue will hold
#define BUFFER_SIZE 1024

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

class ActiveObject {
public:
    using Task = std::function<void(Tree tree, int client_fd)>;

    ActiveObject(Task task) : task_(task), stop_(false) {
        thread_ = std::thread(&ActiveObject::run, this);
    }

    ~ActiveObject() {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            stop_ = true;
            cv_.notify_all();
        }
        thread_.join();
    }

    void send(Tree tree, int client_fd) {
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.emplace(tree, client_fd);
        cv_.notify_one();
    }

private:
    void run() {
        while (true) {
            std::pair<Tree, int> data;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                cv_.wait(lock, [this]() { return stop_ || !queue_.empty(); });
                if (stop_ && queue_.empty()) return;
                data = queue_.front();
                queue_.pop();
            }
            task_(data.first, data.second);
        }
    }

    Task task_;
    std::thread thread_;
    std::queue<std::pair<Tree, int>> queue_;  // The function arguments
    std::mutex mutex_;
    std::condition_variable cv_;
    bool stop_;
};

class Pipeline {
public:
    void addStage(ActiveObject::Task task) {
        stages_.emplace_back(std::make_shared<ActiveObject>(task));
    }

    void execute(Tree tree, int client_fd) {
        for (size_t i = 0; i < stages_.size(); ++i) {
            stages_[i]->send(tree, client_fd);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));   // To ensure they are sent in order
        }
    }

private:
    std::vector<std::shared_ptr<ActiveObject>> stages_;
};

// Each client runs this method independantly (threads)
void handle_client(int client_fd) {
    std::cout << "Handling request..." << std::endl;

    char buffer[1024];
    std::string command = "";
    Graph graph(5); // Default graph with 5 vertices
    Tree mst;
    while (true) { 
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);  // Receive command from client
        if (bytes_received <= 0) {
            cout << "test" << endl;
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
        // Print the current graph
        else if (action == "print_graph") {
            std::ostringstream oss;
            graph.printGraph(oss);  // Assuming printGraph can accept an ostream
            std::string result = oss.str();
            send(client_fd, result.c_str(), result.length(), 0);
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
        
            cout << "The MST : \n";
            mst.printTree();  
            
            Pipeline pipeline;
            // Add stages
            pipeline.addStage(calculateTotalWeight);
            pipeline.addStage(calculateLongestDistance);
            pipeline.addStage(calculateAverageDistance);
            pipeline.addStage(calculateShortestDistance);
            // Execute command
            pipeline.execute(mst, client_fd);
                
            //delete mst_algo;

        }
    }
}

// Stage 1: Receive Request
/* accepts incoming client connections and pushes the client file descriptors into the requestQueue.
 This stage is responsible for receiving requests from clients.
*/
/*accepts new client connections and pushes the client file descriptor (new_fd) into the 
    requestQueue. This represents a new connection from a client, and this descriptor is 
    what will be processed later.
    */

// Stage 2: Process Commands
/* waits for requests to be available in requestQueue, pops a request, and processes it. 
It reads commands from the client, performs various actions (MST computation), and sends the response back to the client.
*/
/*When a client file descriptor is available, it pops the client_fd from 
    the queue and uses it to communicate with the client 
    */
// Active Object for processing commands

// Main function
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

    while (true) {
        sin_size = sizeof their_addr;
        int new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
        cout << "server: got connection from " << s << endl;

        handle_client(new_fd);
        close(new_fd);
    }

    close(sockfd);  // Ensure socket is closed before exit
    return 0;
}
