
CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Iinclude

SRC   := $(wildcard src/*.cpp)
OBJ   := $(SRC:.cpp=.o)
BUILD := build

.PHONY: all clean test run

all: $(BUILD)/formula2bdd

$(BUILD):
	@mkdir -p $(BUILD)

$(BUILD)/formula2bdd: $(SRC) app/main.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) $(SRC) app/main.cpp -o $@

$(BUILD)/test_formula2bdd: $(SRC) tests/test_formula2bdd.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) $(SRC) tests/test_formula2bdd.cpp -o $@

test: $(BUILD)/test_formula2bdd
	./$(BUILD)/test_formula2bdd

run: all
	./$(BUILD)/formula2bdd "(p | (q & r)) -> ~s" --format dot --stats

clean:
	rm -rf $(BUILD)
