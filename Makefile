# Makefile for tane
# Assumptions:
# - All C sources live under src/
# - Header files are under src/ (included via -Isrc)
# - Object files are placed under build/obj/
# - Final executable is bin/tane

# Tools
CC      ?= cc

# Flags
CSTD    ?= c17
CFLAGS  ?= -O2 -g -Wall -Wextra -Wpedantic -std=$(CSTD)
LDFLAGS ?=
LDLIBS  ?=
INCLUDES?= -Isrc

# Directories
SRC_DIR   := src
OBJ_DIR   := build/obj
BIN_DIR   := build
TARGET    := $(BIN_DIR)/tane

# Sources and objects
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

.PHONY: all clean run dirs

all: $(TARGET)

# Link final binary
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@
	@echo "Built $@"

# Compile each source to object with dependency files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

# Ensure object directory exists
$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

# Run the program
run: $(TARGET)
	$(TARGET)

# Clean build outputs
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# Include auto-generated dependency files if present
-include $(DEPS)
