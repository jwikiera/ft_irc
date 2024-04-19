# Compiler
CXX = c++

# Compiler flags
CXXFLAGS = -std=c++98 -Wall -Werror -Wextra -pedantic -g3

# Source files
SOURCES = main.cpp Server.cpp Client.cpp

# Header files
HEADERS = Server.h Client.h responses.h

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Executable name
EXECUTABLE = ircserv

# Make all
all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS)

fclean: clean
	rm -f $(EXECUTABLE)

re: clean all
