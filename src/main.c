//#define NDEBUG
#include <stdio.h>

#include <libdragon.h>

#include "maze.h"

#define ANIMATION_DELAY 40 // in milliseconds

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

#define MAZE_WIDTH 32
#define MAZE_HEIGHT 22
#define BLOCK_SIZE 5

// pages as seen by the UID
enum Page {
        PAGE_INTRO = 0,
        PAGE_MAZE = 1,
};

Maze *gMaze = NULL;
unsigned long gAnimationCounter = 0;
sprite_t *gIntroSprite = NULL;
enum Page gCurrentPage = PAGE_INTRO;

int gMazeSpeed = 3;
int gMazeAnimationDelay = 40;

// get total amount of milliseconds the n64 has been powered on
static inline unsigned long get_total_ms(void) {
    return (timer_ticks() / (TICKS_PER_SECOND / 1000));
}

uint32_t get_color(int x, int y) {
	int r = x * 100 / SCREEN_WIDTH * 255 / 100;
	int g = y * 100 / SCREEN_HEIGHT * 255 / 100;
	int b = 128;
	return graphics_make_color(r, g, b, 0xff);
}

void set_delay() {
	debugf("speed set to %d\n", gMazeSpeed);
	switch (gMazeSpeed) {
		case 1: gMazeAnimationDelay = 200; break;
		case 2: gMazeAnimationDelay = 120; break;
		case 3: gMazeAnimationDelay = 80; break;
		case 4: gMazeAnimationDelay = 40; break;
		case 5: gMazeAnimationDelay = 20; break;
		case 6: gMazeAnimationDelay = 6; break;
	}
}

void increase_speed() {
	gMazeSpeed++;
	if (gMazeSpeed > 6) {
		gMazeSpeed = 6;
	}
	set_delay();
}

void decrease_speed() {
	gMazeSpeed--;
	if (gMazeSpeed < 1) {
		gMazeSpeed = 1;
	}
	set_delay();
}

void display_intro_page(unsigned long delta) {
	surface_t *disp;

	uint32_t black_color = graphics_make_color(0x00, 0x00, 0x00, 0xff);

	// check for input
	controller_scan();
	struct controller_data keys = get_keys_down();

	if (keys.c[0].start) {
		debugf("intro: start pressed - loading maze page\n");
		maze_destroy(gMaze);
		gMaze = maze_create(MAZE_WIDTH, MAZE_HEIGHT);
		gCurrentPage = PAGE_MAZE;
		return;
	}

	while ((disp = display_lock()) == NULL);

	graphics_fill_screen(disp, black_color);
	graphics_draw_sprite(disp, 0, 0, gIntroSprite);

	display_show(disp);
}

void display_maze_page(unsigned long delta) {
	surface_t *disp;

	uint32_t black_color = graphics_make_color(0x00, 0x00, 0x00, 0xff);

	// check for input
	controller_scan();
	struct controller_data keys = get_keys_down();

	if (keys.c[0].A) {
		// reset the maze if A is pressed
		debugf("maze: A pressed - recreating maze\n");
		maze_destroy(gMaze);
		gMaze = maze_create(MAZE_WIDTH, MAZE_HEIGHT);
	} else if (keys.c[0].B) {
		// back out to the intro page
		debugf("maze: B pressed - loading intro page\n");
		gCurrentPage = PAGE_INTRO;
		maze_destroy(gMaze);
		gMaze = NULL;
		return;
	} else if (keys.c[0].C_up) {
		increase_speed();
	} else if (keys.c[0].C_down) {
		decrease_speed();
	}

	// check if we need to advance the maze
	gAnimationCounter += delta;
	while (gAnimationCounter > gMazeAnimationDelay) {
		maze_step(gMaze);
		gAnimationCounter -= gMazeAnimationDelay;
	}

	// draw the graphics
	while ((disp = display_lock()) == NULL);
	graphics_fill_screen(disp, black_color);

	// draw the maze
	for (int i = 0; i < gMaze->height; i++) {
		for (int j = 0; j < gMaze->width; j++) {
			MazeBlock *block = gMaze->grid[i][j];

			int pad = BLOCK_SIZE / 2;
			if (block->sides_seen > 0) {
				int x = pad + j * BLOCK_SIZE * 2;
				int y = pad + i * BLOCK_SIZE * 2;
				uint32_t color = get_color(x, y);
				graphics_draw_box(disp, x, y,
				    BLOCK_SIZE, BLOCK_SIZE, color);
			}
			if (block->sides_seen & MAZE_NORTH) {
				int x = pad + j * BLOCK_SIZE * 2;
				int y = pad + i * BLOCK_SIZE * 2 - BLOCK_SIZE;
				uint32_t color = get_color(x, y);
				graphics_draw_box(disp, x, y,
				    BLOCK_SIZE, BLOCK_SIZE, color);
			}
			if (block->sides_seen & MAZE_SOUTH) {
				int x = pad + j * BLOCK_SIZE * 2;
				int y = pad + i * BLOCK_SIZE * 2 + BLOCK_SIZE;
				uint32_t color = get_color(x, y);
				graphics_draw_box(disp, x, y,
				    BLOCK_SIZE, BLOCK_SIZE, color);
			}
			if (block->sides_seen & MAZE_EAST) {
				int x = pad + j * BLOCK_SIZE * 2 + BLOCK_SIZE;
				int y = pad + i * BLOCK_SIZE * 2;
				uint32_t color = get_color(x, y);
				graphics_draw_box(disp, x, y,
				    BLOCK_SIZE, BLOCK_SIZE, color);
			}
			if (block->sides_seen & MAZE_WEST) {
				int x = pad + j * BLOCK_SIZE * 2 - BLOCK_SIZE;
				int y = pad + i * BLOCK_SIZE * 2;
				uint32_t color = get_color(x, y);
				graphics_draw_box(disp, x, y,
				    BLOCK_SIZE, BLOCK_SIZE, color);
			}

		}
	}

	// draw the status bar
	char buf[100];
	sprintf(buf, "speed %d/6", gMazeSpeed);
	graphics_draw_text(disp, 5, 220, buf);

	display_show(disp);
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

	// play the music
	wav64_t wav;
	wav64_open(&wav, "rom:/lost-corridors.wav64");
	wav64_play(&wav, 0);

	int fp = dfs_open("/intro.sprite");
	gIntroSprite = malloc(dfs_size(fp));
	dfs_read(gIntroSprite, 1, dfs_size(fp), fp);
	dfs_close(fp);

	unsigned long then = get_total_ms();
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

		// handle page-specific logic
		switch (gCurrentPage) {
			case PAGE_INTRO: display_intro_page(delta); break;
			case PAGE_MAZE: display_maze_page(delta); break;
		}
	}
}
