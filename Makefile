CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -O2
TARGET := alias_method
SRC := code2.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean

