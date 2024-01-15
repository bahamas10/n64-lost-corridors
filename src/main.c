#define NDEBUG
#include <stdio.h>

#include <libdragon.h>

#include "maze.h"

#define ANIMATION_DELAY 40 // in milliseconds

#define MAZE_WIDTH 32
#define MAZE_HEIGHT 24
#define BLOCK_SIZE 5

// get total amount of milliseconds the n64 has been powered on
static inline unsigned long get_total_ms(void)
{
    return (timer_ticks() / (TICKS_PER_SECOND / 1000));
}

// main
int main(void) {
	// init console
	console_init();

	// init debug
	debug_init_usblog();
	debug_init_isviewer();
	console_set_debug(true);

	// init internal systems
	timer_init();
	display_init(RESOLUTION_320x240, DEPTH_32_BPP, 3, GAMMA_NONE,
	    FILTERS_RESAMPLE);
	controller_init();
	dfs_init(DFS_DEFAULT_LOCATION);

	// init audio
	audio_init(44100, 20);
	mixer_init(32);
	mixer_ch_set_limits(6, 0, 128000, 0);

	// init rand
	srand(get_ticks());

	// display
	surface_t *disp;

	uint32_t black_color = graphics_make_color(0x00, 0x00, 0x00, 0xff);
	uint32_t red_color = graphics_make_color(0x99, 0x00, 0x00, 0xff);

	// play the music
	wav64_t wav;
	wav64_open(&wav, "rom:/lost-corridors.wav64");
	wav64_play(&wav, 0);

	// create a maze
	Maze *maze = maze_create(MAZE_WIDTH, MAZE_HEIGHT);

	unsigned long then = get_total_ms();
	unsigned long animation_counter = 0;
	while (true) {
		unsigned long now = get_total_ms();
		unsigned long delta = now - then;
		then = now;

		// play the song
		if (audio_can_write()) {
			short *buf = audio_write_begin();
			mixer_poll(buf, audio_get_buffer_length());
			audio_write_end();
		}

		// check for input
		controller_scan();
                struct controller_data keys = get_keys_down();

		if (keys.c[0].A) {
			// reset the maze if A is pressed
			maze_destroy(maze);
			maze = maze_create(MAZE_WIDTH, MAZE_HEIGHT);
		}

		// check if we need to advance the maze
		animation_counter += delta;
		while (animation_counter > ANIMATION_DELAY) {
			maze_step(maze);
			animation_counter -= ANIMATION_DELAY;
		}

		// draw the graphics
		while ((disp = display_lock()) == NULL);
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
					int x = pad + j * BLOCK_SIZE * 2;
					int y = pad + i * BLOCK_SIZE * 2 + BLOCK_SIZE;
					graphics_draw_box(disp, x, y,
					    BLOCK_SIZE, BLOCK_SIZE, red_color);
				}
				if (block->sides_seen & MAZE_EAST) {
					int x = pad + j * BLOCK_SIZE * 2 + BLOCK_SIZE;
					int y = pad + i * BLOCK_SIZE * 2;
					graphics_draw_box(disp, x, y,
					    BLOCK_SIZE, BLOCK_SIZE, red_color);
				}
				if (block->sides_seen & MAZE_WEST) {
					int x = pad + j * BLOCK_SIZE * 2 - BLOCK_SIZE;
					int y = pad + i * BLOCK_SIZE * 2;
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
