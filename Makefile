V=1
SOURCE_DIR=src
BUILD_DIR=build
include $(N64_INST)/include/n64.mk

all: lost-corridors.z64
.PHONY: all

OBJS = $(BUILD_DIR)/main.o $(BUILD_DIR)/maze.o

lost-corridors.z64: N64_ROM_TITLE="Lost Corridors"

$(BUILD_DIR)/lost-corridors.elf: $(OBJS)

clean:
	rm -f $(BUILD_DIR)/* *.z64
.PHONY: clean

-include $(wildcard $(BUILD_DIR)/*.d)
