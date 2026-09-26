# CHIP-8 emulator build
#
#   make            build ./chip8
#   make run        build, then run with ROM=... (default: IBM logo)
#   make debug      build with address sanitizer (catches out-of-bounds)
#   make strict     build with -Wconversion (catches silent narrowing)
#   make clean      delete build artefacts

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wimplicit-fallthrough -g
TARGET   := chip8
SRC_DIR  := src
BUILD_DIR:= build
ROM      ?= roms/2-ibm-logo.ch8

# raylib, found via pkg-config (install with: brew install raylib).
# The include path is passed as -isystem instead of -I so warnings from
# inside raylib.h don't show up alongside warnings from our own code.
RAYLIB_INC  := $(patsubst -I%,-isystem %,$(shell pkg-config --cflags raylib))
RAYLIB_LIBS := $(shell pkg-config --libs raylib)

# Every .cpp in src/ becomes a .o in build/
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

# -MMD -MP makes the compiler emit a .d file listing the headers each .cpp
# includes, so editing a header rebuilds the files that use it.
DEPS := $(OBJS:.o=.d)

.PHONY: all run debug strict clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(RAYLIB_LIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(RAYLIB_INC) -MMD -MP -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

run: $(TARGET)
	./$(TARGET) $(ROM)

# Address sanitizer: reports out-of-bounds reads and writes at runtime.
# Useful when sprite drawing goes wrong.
debug: CXXFLAGS += -fsanitize=address -fno-omit-frame-pointer
debug: clean $(TARGET)

# Warns on every implicit narrowing, e.g. uint16_t assigned into uint8_t.
strict: CXXFLAGS += -Wconversion -Wsign-conversion
strict: clean $(TARGET)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

-include $(DEPS)
