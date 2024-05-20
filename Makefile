# Compiler
CXX = c++

# Compiler flags
CXXFLAGS = -std=c++98 -Wall -Werror -Wextra -pedantic -g3 -Wno-unused-parameter -Wno-unused-private-field # -fsanitize=address

SRC_FOLDER = src
HEADER_FOLDER = headers

# Source files
SOURCES_LIST = main.cpp Server.cpp Client.cpp Util.cpp Channel.cpp

# Header files
HEADERS_LIST = Server.h Client.h responses.h Util.h Channel.h

SOURCES = $(addprefix $(SRC_FOLDER)/, $(SOURCES_LIST))

HEADERS = $(addprefix $(HEADER_FOLDER)/, $(HEADERS_LIST))

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Executable name
EXECUTABLE = ircserv

# Make all
all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -I$(HEADER_FOLDER) -o $@

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -I$(HEADER_FOLDER) -c $< -o $@

clean:
	rm -f $(OBJECTS)

fclean: clean
	rm -f $(EXECUTABLE)

re: clean all
