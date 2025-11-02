# Makefile for tane
# Assumptions:
# - All C++ sources live under src/
# - Header files are under src/ (included via -Isrc)
# - Object files are placed under build/obj/
# - Final executable is build/tane

# Tools
CXX     ?= g++

# Flags
CXXSTD  ?= c++20
CXXFLAGS?= -O0 -g3 -Wall -Wextra -Wpedantic -std=$(CXXSTD)
LDFLAGS ?=
LDLIBS  ?=
INCLUDES?= -Isrc

# Directories
SRC_DIR   := src
OBJ_DIR   := build/obj
BIN_DIR   := build
TARGET    := $(BIN_DIR)/tane

# Sources and objects
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

.PHONY: all clean run dirs test

all: $(TARGET)

# ===== Standard library (assemble std/src/std.asm -> std/lib/obj, archive libstd.a) =====
STD_SRC_DIR := std/src
STD_OBJ_DIR := std/lib/obj
STD_ASM     := $(STD_SRC_DIR)/std.asm
STD_O       := $(STD_OBJ_DIR)/std.o
STD_LIB     := $(STD_OBJ_DIR)/libstd.a

.PHONY: std
std: $(STD_LIB)

$(STD_OBJ_DIR):
	@mkdir -p $(STD_OBJ_DIR)

# Assemble std.asm
$(STD_O): $(STD_ASM) | $(STD_OBJ_DIR)
	$(CXX) -c -x assembler $< -o $@

# Archive static library in std/lib/obj
$(STD_LIB): $(STD_O)
	ar rcs $@ $^
    

# Link final binary
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@
	@echo "Built $@"

# Compile each source to object with dependency files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

# Ensure object directory exists
$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

# Run the program
run: $(TARGET)
	$(TARGET)

# Test target
# Executes test.sh which is expected to run: build/tane "code"
# Make sure test.sh is executable
TEST_SCRIPT := ./test.sh

test: $(TARGET) $(TEST_SCRIPT)
	@chmod +x $(TEST_SCRIPT)
	$(TEST_SCRIPT)

# ===== Build test/src/main.tn and link with standard library =====
TEST_SRC      := test/src/main.tn
TEST_BIN_DIR  := test/build/bin
TEST_S        := $(TEST_BIN_DIR)/main.s
TEST_OBJ      := $(TEST_BIN_DIR)/main.o
TEST_EXE      := test/build/test.exe

.PHONY: test2
test2: $(TARGET) std $(TEST_EXE)
	@echo "Built $(TEST_EXE)"

$(TEST_BIN_DIR):
	@mkdir -p $(TEST_BIN_DIR)

# Compile Tane source to assembly
$(TEST_S): $(TARGET) $(TEST_SRC) | $(TEST_BIN_DIR)
	$(TARGET) $(TEST_SRC) -o $(TEST_S)

# Assemble test assembly to object
$(TEST_OBJ): $(TEST_S)
	$(CXX) -c $< -o $@

# Link with standard library static archive
$(TEST_EXE): $(TEST_OBJ) $(STD_LIB)
	$(CXX) -no-pie -o $@ $^ 

# Clean build outputs
clean:
	rm -rf $(OBJ_DIR) $(TARGET) $(STD_OBJ_DIR) test/build

# Keep intermediate assembly file
.PRECIOUS: $(TEST_S)

# Include auto-generated dependency files if present
-include $(DEPS)
