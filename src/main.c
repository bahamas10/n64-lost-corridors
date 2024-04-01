#define NDEBUG
#include <stdio.h>

#include <libdragon.h>

#include "maze.h"
#include "ryb2rgb.h"

#define DEFAULT_MAZE_SPEED 3 // init speed - can be 1-6 inclusive
#define SPEED_DIALOG_SHOW_TIME (2 * 1000 + 500) // 2.5 seconds

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// you can change these - but a lot of the code is built around the maze being
// divisible by the screen size so it could result in weird things happening if
// you do.
#define MAZE_WIDTH 32
#define MAZE_HEIGHT 24
#define BLOCK_SIZE 5

// "scenes" for the game
enum Scene {
	SCENE_MAIN_MENU = 0,
	SCENE_MAZE = 1,
};

// the current scene we are displaying
enum Scene gCurrentScene;

// the current maze object itself from maze.c
Maze *gMaze = NULL;

// maze animation and speed variables
int gMazeAnimationCounter = 0;
int gMazeSpeed = 3;
int gMazeAnimationDelay;

// how long to show the "speed x/6" dialog
int gStatusAnimationCounter = 0;

// the intro menu sprite object (loaded once and forever held in memory)
sprite_t *gIntroSprite = NULL;

// random color mode related variables
float gRandomMagic[8][3];
bool gColorEnabled = false;

// function prototypes
void switch_scene(enum Scene scene);

// get total amount of milliseconds the n64 has been powered on
static inline unsigned long get_total_ms(void) {
	return (timer_ticks() / (TICKS_PER_SECOND / 1000));
}

// randomize the colors used by ryb2rgb
void randomize_magic() {
	debugf("colors randomized\n");
        for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 3; j++) {
                        gRandomMagic[i][j] = (float)rand()/(float)RAND_MAX;
                }
        }
}

// given a location on screen figure out the color to use for it
uint32_t get_color(int x, int y) {
	int r, g, b;

	if (gColorEnabled) {
		r = x * 100 / SCREEN_WIDTH * 255 / 100;
		g = y * 100 / SCREEN_HEIGHT * 255 / 100;
		b = 128;

		RGB color = interpolate2rgb(r / 255.0, g / 255.0, b / 255.0, gRandomMagic);
		r = color.r * 255;
		g = color.g * 255;
		b = color.b * 255;
	} else {
		// static color - taken from
		// https://www.youtube.com/watch?v=RCjLs9koZQg
		r = 110;
		g = 78;
		b = 40;
	}

	return graphics_make_color(r, g, b, 0xff);
}

// set the speed
void set_speed(int speed) {
	gMazeSpeed = speed;
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

// increase the speed by one (or no-op if already max)
void increase_speed() {
	int speed = gMazeSpeed;
	if (speed < 6) {
		speed++;
	}
	set_speed(speed);
}

// decrease the speed by one (or no-op if already min)
void decrease_speed() {
	int speed = gMazeSpeed;
	if (speed > 1) {
		speed--;
	}
	set_speed(speed);
}

// called when the menu scene is about be displayed
void init_main_menu_scene() {
	maze_destroy(gMaze);
	gMaze = NULL;
}

// display logic for the intro menu Scene
void render_main_menu_scene(unsigned long delta) {
	surface_t *disp;

	uint32_t black_color = graphics_make_color(0x00, 0x00, 0x00, 0xff);

	// check for input
	controller_scan();
	struct controller_data keys = get_keys_down();

	if (keys.c[0].start) {
		debugf("main-menu: start pressed - loading maze scene\n");

		switch_scene(SCENE_MAZE);
		return;
	}

	while ((disp = display_lock()) == NULL);

	graphics_fill_screen(disp, black_color);
	graphics_draw_sprite(disp, 0, 0, gIntroSprite);

	display_show(disp);
}

// called when the maze scene is about to be displayed
void init_maze_scene() {
	// reset the maze
	maze_destroy(gMaze);
	gMaze = maze_create(MAZE_WIDTH, MAZE_HEIGHT);

	// reset the animation counters
	gStatusAnimationCounter = 0;

	// reset colors and speed
	gColorEnabled = false;
	set_speed(DEFAULT_MAZE_SPEED);
}

// called when the maze scene is being rendered (every frame)
void render_maze_scene(unsigned long delta) {
	surface_t *disp;

	uint32_t black_color = graphics_make_color(0x00, 0x00, 0x00, 0xff);

	// check for input
	controller_scan();
	struct controller_data keys = get_keys_down();

	if (keys.c[0].A) {
		debugf("maze: A pressed - recreating maze\n");

		// recreate the maze
		maze_destroy(gMaze);
		gMaze = maze_create(MAZE_WIDTH, MAZE_HEIGHT);
	} else if (keys.c[0].B) {
		debugf("maze: B pressed - loading main menu scene\n");

		// load the main menu scene
		switch_scene(SCENE_MAIN_MENU);
		return;
	} else if (keys.c[0].C_up) {
		// increase the speed and show the animation

		increase_speed();
		gStatusAnimationCounter = SPEED_DIALOG_SHOW_TIME;
	} else if (keys.c[0].C_down) {
		// decrease the speed and show the animation

		decrease_speed();
		gStatusAnimationCounter = SPEED_DIALOG_SHOW_TIME;
	} else if (keys.c[0].C_left) {
		// disable random colors
		gColorEnabled = false;
	} else if (keys.c[0].C_right) {
		// enable random colors
		gColorEnabled = true;
		randomize_magic();
	}

	// check if we need to advance the maze
	gMazeAnimationCounter += delta;
	while (gMazeAnimationCounter > gMazeAnimationDelay) {
		maze_step(gMaze);
		gMazeAnimationCounter -= gMazeAnimationDelay;
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

	// check if we need to draw the speed status
	gStatusAnimationCounter -= delta;
	if (gStatusAnimationCounter < 0) {
		gStatusAnimationCounter = 0;
	}
	if (gStatusAnimationCounter > 0) {
		// draw the status bar
		char buf[100];
		sprintf(buf, "speed %d/6", gMazeSpeed);
		graphics_draw_text(disp, 18, 18, buf);
	}

	display_show(disp);
}

// called to switch active scene
void switch_scene(enum Scene scene) {
	gCurrentScene = scene;
	switch (scene) {
		case SCENE_MAIN_MENU: init_main_menu_scene(); break;
		case SCENE_MAZE: init_maze_scene(); break;
	}
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

	// load the main menu sprite
	int fp = dfs_open("/intro.sprite");
	gIntroSprite = malloc(dfs_size(fp));
	dfs_read(gIntroSprite, 1, dfs_size(fp), fp);
	dfs_close(fp);

	// init the magic values
	randomize_magic();

	// init the default scene
	switch_scene(SCENE_MAIN_MENU);

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

		// handle scene-specific logic
		switch (gCurrentScene) {
			case SCENE_MAIN_MENU: render_main_menu_scene(delta); break;
			case SCENE_MAZE: render_maze_scene(delta); break;
		}
	}
}
