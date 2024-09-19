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

# Object files
OBJ_MAIN = $(SRC_MAIN:.cpp=.o)
OBJ_SERVER = $(SRC_SERVER:.cpp=.o)

# Executables
EXEC_MAIN = main
EXEC_SERVER = server

# Default target
all: $(EXEC_MAIN) $(EXEC_SERVER)

# Build the executables
$(EXEC_MAIN): $(OBJ_MAIN)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

$(EXEC_SERVER): $(OBJ_SERVER)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

# Compile source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run the main program
run_main: $(EXEC_MAIN)
	./$(EXEC_MAIN) -v 5 -e 7 -s 42

# Run the server
run_server: $(EXEC_SERVER)
	./$(EXEC_SERVER)

# Generate code coverage report
coverage: CXXFLAGS += $(COVFLAGS)
coverage: clean $(EXEC_SERVER)
	./$(EXEC_SERVER) -v 5 -e 7 -s 42  # Run the server with necessary arguments
	gcov $(SRC_SERVER)                 # Change this to point to your server source files
	lcov --capture --directory . --output-file coverage.info
	genhtml coverage.info --output-directory out
#  gcov your_server_source_file.cpp

# Profiling with gprof
profile: CXXFLAGS += $(PROFFLAGS)
profile: clean $(EXEC_SERVER)
	./$(EXEC_SERVER) -v 5 -e 7 -s 42  # Run the server with necessary arguments
	gprof $(EXEC_SERVER) gmon.out > analysis.txt

# Memory checking with Valgrind (memcheck)
valgrind: $(EXEC_SERVER)
	valgrind $(VALFLAGS) ./$(EXEC_SERVER)

# Memory checking with Valgrind (helgrind)
valgrind_helgrind: $(EXEC_SERVER)
	valgrind --tool=helgrind --log-file="valgrind_helgrind_log.txt" ./$(EXEC_SERVER)

# Generate call graph with Valgrind's callgrind tool
valgrind_callgrind: $(EXEC_SERVER)
	valgrind --tool=callgrind --callgrind-out-file=custom_callgrind.out ./$(EXEC_SERVER)
	kcachegrind custom_callgrind.out

# Clean up generated files
clean:
	rm -f *.o $(EXEC_MAIN) $(EXEC_SERVER) gmon.out *.gcda *.gcno *.gcov coverage.info 
	rm -rf out
	rm -f valgrind_log.txt valgrind_helgrind_log.txt custom_callgrind.out

.PHONY: all run_main run_server coverage profile valgrind valgrind_memcheck valgrind_callgrind clean