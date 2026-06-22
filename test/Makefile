#============================================================================
# Makefile — Host-based unit tests for FreeRTOS_mutask_dataacq
# Builds a native x86_64 executable using gcc + Unity framework.
#============================================================================

CC      := gcc
CFLAGS  := -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-function
CFLAGS  += -std=c11 -g -O0
LDFLAGS := -lm

# Directories
UNITY_DIR    := lib
MOCK_DIR     := mocks
BSP_INC_DIR  := ../BSP/Inc
CORE_INC_DIR := ../Core/Inc

# Source files
UNITY_SRC    := $(UNITY_DIR)/unity.c
TEST_SRCS    := test_data_process.c test_runner.c
ALL_SRCS     := $(UNITY_SRC) $(TEST_SRCS)

# Include paths
INCLUDES := -I. -I$(UNITY_DIR) -I$(MOCK_DIR) -I$(BSP_INC_DIR) -I$(CORE_INC_DIR)

# Target
TARGET := test_data_process

#============================================================================
# Build rules
#============================================================================
.PHONY: all test clean

all: $(TARGET)

$(TARGET): $(ALL_SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS)

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
