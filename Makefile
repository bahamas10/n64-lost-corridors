V=1
SOURCE_DIR=src
BUILD_DIR=build
include $(N64_INST)/include/n64.mk

all: lost-corridors.z64
.PHONY: all

OBJS = $(BUILD_DIR)/main.o $(BUILD_DIR)/maze.o $(BUILD_DIR)/ryb2rgb.o

lost-corridors.z64: N64_ROM_TITLE="Lost Corridors"
lost-corridors.z64: $(BUILD_DIR)/lost-corridors.dfs

$(BUILD_DIR)/lost-corridors.elf: $(OBJS)
$(BUILD_DIR)/lost-corridors.dfs: $(wildcard filesystem/*)

clean:
	rm -f $(BUILD_DIR)/* *.z64
.PHONY: clean

-include $(wildcard $(BUILD_DIR)/*.d)
