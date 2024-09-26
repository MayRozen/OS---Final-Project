# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Werror -Wsign-conversion -g

# Coverage flags
COVFLAGS = -fprofile-arcs -ftest-coverage

# Profiling flags
PROFFLAGS = -pg

# Valgrind flags
VALFLAGS = --leak-check=full --log-file="valgrind_log.txt"

# Linker flags for Boost libraries
LDFLAGS = -lboost_system

# Source files
SRC_MAIN = main.cpp Graph.cpp Tree.cpp MSTFactory.cpp KruskalMST.cpp PrimMST.cpp BoruvkaMST.cpp TarjanMST.cpp IntegerMST.cpp
SRC_SERVER = Server.cpp Graph.cpp Tree.cpp MSTFactory.cpp KruskalMST.cpp PrimMST.cpp BoruvkaMST.cpp TarjanMST.cpp IntegerMST.cpp
SRC_SERVER_PIPE = serverPipe.cpp Graph.cpp Tree.cpp MSTFactory.cpp KruskalMST.cpp PrimMST.cpp BoruvkaMST.cpp TarjanMST.cpp IntegerMST.cpp

# Object files
OBJ_MAIN = $(SRC_MAIN:.cpp=.o)
OBJ_SERVER = $(SRC_SERVER:.cpp=.o)
OBJ_SERVER_PIPE = $(SRC_SERVER_PIPE:.cpp=.o)

# Executables
EXEC_MAIN = main
EXEC_SERVER = server
EXEC_SERVER_PIPE = serverPipe

# Default target
all: $(EXEC_MAIN) $(EXEC_SERVER) $(EXEC_SERVER_PIPE)

# Build the executables
$(EXEC_MAIN): $(OBJ_MAIN)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

$(EXEC_SERVER): $(OBJ_SERVER)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

$(EXEC_SERVER_PIPE): $(OBJ_SERVER_PIPE)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

# Compile source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run the main program
run_main: $(EXEC_MAIN)
	./$(EXEC_MAIN) -v 5 -e 7 -s 42

# Run the servers
run_server: $(EXEC_SERVER)
	./$(EXEC_SERVER)

run_serverPipe: $(EXEC_SERVER_PIPE)
	./$(EXEC_SERVER_PIPE)

# Generate code coverage report
coverage: CXXFLAGS += $(COVFLAGS)
coverage: clean $(EXEC_SERVER_PIPE)
	./$(EXEC_SERVER_PIPE) -v 5 -e 7 -s 42  # Run the server with necessary arguments
	gcov $(SRC_SERVER_PIPE)                 # Change this to point to your serverPipe source files
	lcov --capture --directory . --output-file coverage.info
	genhtml coverage.info --output-directory out

# Profiling with gprof
profile: CXXFLAGS += $(PROFFLAGS)
profile: clean $(EXEC_SERVER_PIPE)
	./$(EXEC_SERVER_PIPE) -v 5 -e 7 -s 42  # Run the server with necessary arguments
	gprof $(EXEC_SERVER_PIPE) gmon.out > analysis.txt

# Memory checking with Valgrind (memcheck)
valgrind: $(EXEC_SERVER_PIPE)
	valgrind $(VALFLAGS) ./$(EXEC_SERVER_PIPE)

# Memory checking with Valgrind (helgrind)
valgrind_helgrind: $(EXEC_SERVER_PIPE)
	valgrind --tool=helgrind --log-file="valgrind_helgrind_log.txt" ./$(EXEC_SERVER_PIPE)

# Generate call graph with Valgrind's callgrind tool
valgrind_callgrind: $(EXEC_SERVER_PIPE)
	valgrind --tool=callgrind --callgrind-out-file=custom_callgrind.out ./$(EXEC_SERVER_PIPE)
	kcachegrind custom_callgrind.out

# Clean up generated files
clean:
	rm -f *.o $(EXEC_MAIN) $(EXEC_SERVER) $(EXEC_SERVER_PIPE) gmon.out *.gcda *.gcno *.gcov coverage.info 
	rm -rf out
	rm -f valgrind_log.txt valgrind_helgrind_log.txt custom_callgrind.out

.PHONY: all run_main run_server run_serverPipe coverage profile valgrind valgrind_memcheck valgrind_callgrind clean
