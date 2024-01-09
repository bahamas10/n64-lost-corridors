#include <stdio.h>

#include <libdragon.h>

#include "maze.h"

#define ANIMATION_DELAY 1 // in milliseconds

#define MAZE_WIDTH 32
#define MAZE_HEIGHT 24
#define BLOCK_SIZE 5

// animation counter
static volatile uint32_t gAnimCounter = 0;

void update_counter(int _ovfl) {
    gAnimCounter++;
}

// main
int main(void) {
	console_init();

	debug_init_usblog();
	debug_init_isviewer();
	console_set_debug(true);

	timer_init();
	display_init(RESOLUTION_320x240, DEPTH_32_BPP, 3, GAMMA_NONE,
	    FILTERS_RESAMPLE);
	controller_init();

	srand(get_ticks());

	// increment the animation counter every ANIMATION_DELAY ms
	new_timer(TIMER_TICKS(ANIMATION_DELAY * 1000), TF_CONTINUOUS,
	    update_counter);

	uint32_t black_color = graphics_make_color(0x00, 0x00, 0x00, 0xff);
	uint32_t red_color = graphics_make_color(0x99, 0x00, 0x00, 0xff);

	// create a maze
	Maze *maze = maze_create(MAZE_WIDTH, MAZE_HEIGHT);

	while (true) {
		controller_scan();
                struct controller_data keys = get_keys_down();

		if (keys.c[0].A) {
			maze_destroy(maze);
			maze = maze_create(MAZE_WIDTH, MAZE_HEIGHT);
		}

		// only do work when the animation conture has incremented
		while (gAnimCounter > 0) {
			maze_step(maze);
			gAnimCounter--;
		}

		display_context_t disp = display_lock();
		graphics_fill_screen(disp, black_color);

		for (int i = 0; i < maze->height; i++) {
			for (int j = 0; j < maze->width; j++) {
				MazeBlock *block = maze->grid[i][j];

				int pad = BLOCK_SIZE / 2;
				if (block->sides_seen > 0) {
					int x = pad + j * BLOCK_SIZE * 2;
					int y = pad + i * BLOCK_SIZE * 2;
					graphics_draw_box(disp, x, y,
					    BLOCK_SIZE, BLOCK_SIZE, red_color);
				}
				if (block->sides_seen & MAZE_NORTH) {
					int x = pad + j * BLOCK_SIZE * 2;
					int y = pad + i * BLOCK_SIZE * 2 - BLOCK_SIZE;
					graphics_draw_box(disp, x, y,
					    BLOCK_SIZE, BLOCK_SIZE, red_color);
				}
				if (block->sides_seen & MAZE_SOUTH) {
					int x = pad +j * BLOCK_SIZE * 2;
					int y = pad +i * BLOCK_SIZE * 2 + BLOCK_SIZE;
					graphics_draw_box(disp, x, y,
					    BLOCK_SIZE, BLOCK_SIZE, red_color);
				}
				if (block->sides_seen & MAZE_EAST) {
					int x = pad +j * BLOCK_SIZE * 2 + BLOCK_SIZE;
					int y = pad +i * BLOCK_SIZE * 2;
					graphics_draw_box(disp, x, y,
					    BLOCK_SIZE, BLOCK_SIZE, red_color);
				}
				if (block->sides_seen & MAZE_WEST) {
					int x = pad +j * BLOCK_SIZE * 2 - BLOCK_SIZE;
					int y = pad +i * BLOCK_SIZE * 2;
					graphics_draw_box(disp, x, y,
					    BLOCK_SIZE, BLOCK_SIZE, red_color);
				}

			}
		}


		display_show(disp);
	}

	// we shouldn't get here - but this is what cleanup would look like
	maze_destroy(maze);
}
