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
    close(sockfd);
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

    // Execute the already compiled binary
    FILE *fp = popen("/home/hadarfro/Desktop/OS---Final-Project/main", "r");
    if (fp == nullptr) {
        const char *error_msg = "Failed to run the executable.\n";
        perror("popen");
        send(client_fd, error_msg, strlen(error_msg), 0);
    } 
    else {
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), fp) != nullptr) {
            send(client_fd, buffer, strlen(buffer), 0);  // Send the output to the client
        }
        pclose(fp);
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
    hints.ai_family = AF_INET;  // Change to AF_INET if you only need IPv4
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

    return 0;
}
