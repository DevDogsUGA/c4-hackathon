CXX ?= g++
CXXFLAGS ?= -O2 -std=c++17 -Wall -Wextra
CPPFLAGS += -isystem vendor
LDFLAGS ?= -pthread

# Every .cpp file in this folder is compiled in, so you can add your own files.
# Each compiles to its own .o, so editing bot.cpp doesn't recompile the
# (slow-to-build) HTTP server.
SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

.PHONY: run clean

bot: $(OBJECTS)
	$(CXX) $(OBJECTS) -o bot $(LDFLAGS)

%.o: %.cpp $(wildcard *.hpp)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

run: bot
	./bot

clean:
	rm -f bot *.o
