#include <stdio.h>

#include <libdragon.h>

#define ANIMATION_DELAY 20 // in milliseconds

#define MAZE_WIDTH 32
#define MAZE_HEIGHT 24
#define BLOCK_SIZE 5

// util
static unsigned int gBytesAllocated = 0;
void *safe_malloc(size_t size, const char *msg) {
        void *ptr = malloc(size);
	assertf(ptr != NULL, "malloc %s", msg);
	gBytesAllocated += size;
	debugf("malloc %s: %u bytes (%u total)\n", msg, size, gBytesAllocated);
        return ptr;
}

// maze stuff

enum Direction {
	EAST = 1,
	WEST = 2,
	NORTH = 4,
	SOUTH = 8,
};

enum Direction opposite_direction(enum Direction dir) {
	switch (dir) {
		case NORTH: return SOUTH;
		case SOUTH: return NORTH;
		case EAST: return WEST;
		case WEST: return EAST;
	};
	assertf(false, "invalid argument for opposite_direction: %u\n", dir);
}

void shuffle_array(enum Direction *array, size_t n) {
	for (int i = n - 1; i > 0; i--) {
		int j = rand() % (i + 1);
		int tmp = array[i];
		array[i] = array[j];
		array[j] = tmp;
	}
}


typedef struct MazeBlock {
	int sides_seen;
} MazeBlock;

typedef struct MazeStep {
	int x;
	int y;
	enum Direction direction;
} MazeStep;

typedef struct Maze {
	int height;
	int width;
	MazeBlock ***grid;
	MazeStep **stack;
	unsigned int stack_ptr;
	unsigned int stack_max;
} Maze;

void maze_step_push(Maze *maze, int x, int y, enum Direction direction) {
	assertf(maze->stack_ptr < maze->stack_max - 1, "maze stack overrun");


	MazeStep *step = safe_malloc(sizeof(MazeStep), "maze_create stack push");
	step->x = x;
	step->y = y;
	step->direction = direction;
	maze->stack[maze->stack_ptr++] = step;

	debugf("stack push: cur len %u\n", maze->stack_ptr);
}

MazeStep *maze_step_pop(Maze *maze) {
	if (maze->stack_ptr == 0) {
		return NULL;
	}

	MazeStep *step = maze->stack[maze->stack_ptr];

	debugf("stack pop: cur len %u\n", maze->stack_ptr);
	maze->stack[maze->stack_ptr] = NULL;
	maze->stack_ptr--;

	return step;
}

Maze *maze_create(int width, int height) {
	Maze *maze = safe_malloc(sizeof(Maze), "maze_create Maze");

	maze->height = height;
	maze->width = width;
	maze->grid = safe_malloc(height * sizeof(int *), "maze_create height");
	maze->stack_ptr = 0;

	for (int i = 0; i < height; i++) {
		maze->grid[i] = safe_malloc(width * sizeof(int *), "maze_create width");
		for (int j = 0; j < width; j++) {
			MazeBlock *block = safe_malloc(sizeof(MazeBlock), "maze_create maze_block");

			block->sides_seen = 0;

			maze->grid[i][j] = block;
		}
	}

	// calculate all possible steps
	maze->stack_max = width * height * 4;
	maze->stack = safe_malloc(maze->stack_max * sizeof(MazeStep *), "maze_create stack");

	// TODO randomize
	maze_step_push(maze, 0, 0, NORTH);
	maze_step_push(maze, 0, 0, SOUTH);
	maze_step_push(maze, 0, 0, WEST);
	maze_step_push(maze, 0, 0, EAST);

	return maze;
}

void maze_debug(Maze *maze) {
	for (int i = 0; i < maze->height; i++) {
		for (int j = 0; j < maze->width; j++) {
			debugf("maze[%d][%d] = %s\n",
			    i, j, maze->grid[i][j]->sides_seen ? "true" : "false");
		}
	}
}

static int dx(enum Direction direction, int x) {
	switch (direction) {
		case EAST: return 1;
		case WEST: return -1;
		case NORTH: return 0;
		case SOUTH: return 0;
	}
	assertf(false, "dx invalid direction");
}

static int dy(enum Direction direction, int y) {
	switch (direction) {
		case EAST: return 0;
		case WEST: return 0;
		case NORTH: return -1;
		case SOUTH: return 1;
	}
	assertf(false, "dy invalid direction");
}

// single step through the maze
void maze_step(Maze *maze) {
	MazeStep *step = maze_step_pop(maze);

	if (step == NULL) {
		// nothing to do - maze is done
		return;
	}

	int cx = step->x;
	int cy = step->y;
	enum Direction direction = step->direction;
	enum Direction oppo_direction = opposite_direction(direction);

	MazeBlock *curBlock = maze->grid[cy][cx];

	int nx = cx + dx(direction, cx);
	int ny = cy + dy(direction, cy);

	free(step);

	if (nx < 0 || nx >= maze->width || ny < 0 || ny >= maze->height) {
		// out of bounds - try again
		debugf("out of bounds\n");
		maze_step(maze);
		return;
	}

	MazeBlock *newBlock = maze->grid[ny][nx];

	if (newBlock->sides_seen) {
		// this block has already been processed - try again
		debugf("already seen\n");
		maze_step(maze);
		return;
	}

	// mark the block as seen
	curBlock->sides_seen |= direction;
	newBlock->sides_seen |= oppo_direction;

	// push sides left to be seen
	enum Direction directions[4] = {
		NORTH,
		SOUTH,
		EAST,
		WEST,
	};
	shuffle_array(directions, 4);
	for (int i = 0; i < 4; i++) {
		maze_step_push(maze, nx, ny, directions[i]);
	}
}

void maze_destroy(Maze *maze) {
	// TODO implement this
	if (maze != NULL) {
		free(maze);
	}
}

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
	srand(get_ticks());

	display_init(RESOLUTION_320x240, DEPTH_32_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);
	timer_init();

	new_timer(TIMER_TICKS(ANIMATION_DELAY * 1000), TF_CONTINUOUS, update_counter);

	uint32_t black_color = graphics_make_color(0x00, 0x00, 0x00, 0xff);
	uint32_t red_color = graphics_make_color(0xff, 0x00, 0x00, 0xff);
	uint32_t link_color = graphics_make_color(0xff, 0x00, 0x00, 0xff);

	Maze *maze = maze_create(MAZE_WIDTH, MAZE_HEIGHT);

	//maze_debug(maze);

	while(1) {
		while (gAnimCounter > 0) {
			maze_step(maze);
			gAnimCounter--;
		}

		display_context_t disp = display_lock();
		graphics_fill_screen(disp, black_color);

		for (int i = 0; i < maze->height; i++) {
			for (int j = 0; j < maze->width; j++) {
				MazeBlock *block = maze->grid[i][j];

				if (block->sides_seen > 0) {
					int x = j * BLOCK_SIZE * 2;
					int y = i * BLOCK_SIZE * 2;
					graphics_draw_box(disp, x, y, BLOCK_SIZE, BLOCK_SIZE, link_color);
				}
				if (block->sides_seen & NORTH) {
					int x = j * BLOCK_SIZE * 2;
					int y = i * BLOCK_SIZE * 2 - BLOCK_SIZE;
					graphics_draw_box(disp, x, y, BLOCK_SIZE, BLOCK_SIZE, red_color);
				}
				if (block->sides_seen & SOUTH) {
					int x = j * BLOCK_SIZE * 2;
					int y = i * BLOCK_SIZE * 2 + BLOCK_SIZE;
					graphics_draw_box(disp, x, y, BLOCK_SIZE, BLOCK_SIZE, red_color);
				}
				if (block->sides_seen & EAST) {
					int x = j * BLOCK_SIZE * 2 + BLOCK_SIZE;
					int y = i * BLOCK_SIZE * 2;
					graphics_draw_box(disp, x, y, BLOCK_SIZE, BLOCK_SIZE, red_color);
				}
				if (block->sides_seen & WEST) {
					int x = j * BLOCK_SIZE * 2 - BLOCK_SIZE;
					int y = i * BLOCK_SIZE * 2;
					graphics_draw_box(disp, x, y, BLOCK_SIZE, BLOCK_SIZE, red_color);
				}

			}
		}


		display_show(disp);
	}

	// we shouldn't get here - but this is what cleanup would look like
	maze_destroy(maze);
}
