export PATH := C:/msys64/ucrt64/bin;$(PATH)

CXX      := C:/msys64/ucrt64/bin/g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Iinclude
LDFLAGS  := -lsqlite3 -lzmq

SRC_DIR   := src
BUILD_DIR := build
TARGET    := football_core.exe

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

run: $(TARGET)
	./$(TARGET)

clean:
	-rmdir /S /Q $(BUILD_DIR) 2>nul || rm -rf $(BUILD_DIR)
	-del /Q $(TARGET) 2>nul || rm -f $(TARGET)

.PHONY: all run clean