# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
INCLUDES = -I/opt/homebrew/include
LIBS = -L/opt/homebrew/lib -lsfml-graphics -lsfml-audio -lsfml-system -lsfml-window

# Directories
SRCDIR = main
OBJDIR = obj
INCDIRS = -I$(SRCDIR)
BINDIR = .

# Files
SOURCES = $(SRCDIR)/main.cpp $(SRCDIR)/Game.cpp
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
TARGET = $(BINDIR)/snake_game

# Default target
all: $(TARGET)

# Create target executable
$(TARGET): $(OBJECTS) | $(BINDIR)
	$(CXX) $(OBJECTS) -o $@ $(LIBS)

# Compile source files to object files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(INCDIRS) -c $< -o $@

# Create directories if they don't exist
$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

# Clean build files
clean:
	rm -rf $(OBJDIR) $(TARGET)

# Install SFML (macOS with Homebrew)
install-deps:
	brew install sfml

# Run the game
run: $(TARGET)
	./$(TARGET)

# Debug build
debug: CXXFLAGS += -g -DDEBUG
debug: $(TARGET)

.PHONY: all clean install-deps run debug
