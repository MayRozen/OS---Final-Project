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
coverage: clean $(EXEC_MAIN) $(EXEC_SERVER)
	./$(EXEC_MAIN) -v 5 -e 7 -s 42
	gcov $(SRC_MAIN)
	lcov --capture --directory . --output-file coverage.info
	genhtml coverage.info --output-directory out

# Profiling with gprof
profile: CXXFLAGS += $(PROFFLAGS)
profile: clean $(EXEC_MAIN) $(EXEC_SERVER)
	./$(EXEC_MAIN) -v 5 -e 7 -s 42
	gprof $(EXEC_MAIN) gmon.out > analysis.txt

# Memory checking with Valgrind
valgrind: $(EXEC_MAIN)
	valgrind $(VALFLAGS) ./$(EXEC_MAIN) -v 5 -e 7 -s 42

# Memory checking with Valgrind (memcheck)
valgrind_memcheck: $(EXEC_MAIN)
	valgrind $(VALFLAGS) ./$(EXEC_MAIN) -v 5 -e 7 -s 42

# Generate call graph with Valgrind's callgrind tool
valgrind_callgrind: $(EXEC_MAIN)
	valgrind --tool=callgrind --callgrind-out-file=custom_callgrind.out ./$(EXEC_MAIN) -v 5 -e 7 -s 42
	kcachegrind custom_callgrind.out

# Clean up generated files
clean:
	rm -f *.o $(EXEC_MAIN) $(EXEC_SERVER) gmon.out *.gcda *.gcno *.gcov coverage.info 
	rm -rf out
	rm -f valgrind_log.txt custom_callgrind.out

.PHONY: all run_main run_server coverage profile valgrind valgrind_memcheck valgrind_callgrind clean
