# LibGodot Test Sample Makefile

# Paths relative to this directory
ROOT_DIR := ../../..
GODOT_DIR := $(ROOT_DIR)/godot
GODOT_CPP_DIR := $(ROOT_DIR)/godot-cpp
BUILD_DIR := $(ROOT_DIR)/build/libgodot_test

# Compiler settings
CXX := g++
CXXFLAGS := -std=c++17 -fPIC -O2

# Include paths
INCLUDES := \
	-I$(GODOT_CPP_DIR)/include \
	-I$(GODOT_CPP_DIR)/gen/include \
	-I$(GODOT_CPP_DIR)/gdextension

# Library paths and libs
LDFLAGS := -L$(GODOT_CPP_DIR)/bin
LIBS := -lgodot-cpp.linux.template_release.x86_64 -ldl

# LibGodot shared library
LIBGODOT_SO := $(GODOT_DIR)/bin/libgodot.linuxbsd.template_release.x86_64.so

# Output
TARGET := $(BUILD_DIR)/libgodot_test

# Source files
SOURCES := main.cpp
OBJECTS := $(SOURCES:%.cpp=$(BUILD_DIR)/%.o)

.PHONY: all clean run

all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) $(LDFLAGS) $(LIBS) -o $@

# Copy libgodot.so and project, then run
run: $(TARGET)
	cp $(LIBGODOT_SO) $(BUILD_DIR)/libgodot.so
	cp -r project $(BUILD_DIR)/
	cd $(BUILD_DIR) && ./libgodot_test

clean:
	rm -rf $(BUILD_DIR)
