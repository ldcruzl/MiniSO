CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -pthread
SRC = $(wildcard src/*.cpp)
OBJ = $(SRC:.cpp=.o)

miniso: $(OBJ)
	$(CXX) $(CXXFLAGS) -o miniso $(OBJ)

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f src/*.o miniso