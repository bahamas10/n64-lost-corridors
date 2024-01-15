#include <stdio.h>

#include <libdragon.h>

#include "maze.h"

// wrapper for malloc that generates an error if it fails
static void *safe_malloc(size_t size, const char *msg) {
        void *ptr = malloc(size);
	assertf(ptr != NULL, "malloc %s", msg);
        return ptr;
}

// very simple array shuffling logic
static void shuffle_array(MazeDirection *array, size_t n) {
	for (int i = n - 1; i > 0; i--) {
		int j = rand() % (i + 1);
		int tmp = array[i];
		array[i] = array[j];
		array[j] = tmp;
	}
}

// return the opposite direction for a given direction
static MazeDirection opposite_direction(MazeDirection dir) {
	switch (dir) {
		case MAZE_NORTH: return MAZE_SOUTH;
		case MAZE_SOUTH: return MAZE_NORTH;
		case MAZE_EAST: return MAZE_WEST;
		case MAZE_WEST: return MAZE_EAST;
	};
	assertf(false, "invalid argument for opposite_direction: %u\n", dir);
}

// figure out the "x" of the neighbor block in the given direction
static int dx(MazeDirection direction, int x) {
	switch (direction) {
		case MAZE_EAST: return 1;
		case MAZE_WEST: return -1;
		case MAZE_NORTH: return 0;
		case MAZE_SOUTH: return 0;
	}
	assertf(false, "dx invalid direction");
}

// figure out the "y" of the neighbor block in the given direction
static int dy(MazeDirection direction, int y) {
	switch (direction) {
		case MAZE_EAST: return 0;
		case MAZE_WEST: return 0;
		case MAZE_NORTH: return -1;
		case MAZE_SOUTH: return 1;
	}
	assertf(false, "dy invalid direction");
}

// push a "step" onto the stack
static void maze_step_push(Maze *maze, int x, int y, MazeDirection direction) {
	assertf(maze->stack_ptr < maze->stack_max, "maze stack overrun");

	MazeStep *step = safe_malloc(sizeof(MazeStep),
	    "maze_create stack push");

	step->x = x;
	step->y = y;
	step->direction = direction;
	maze->stack[maze->stack_ptr] = step;
	maze->stack_ptr++;
}

// pop a "step" off of the stack
static MazeStep *maze_step_pop(Maze *maze) {
	if (maze->stack_ptr == 0) {
		return NULL;
	}
	maze->stack_ptr--;

	MazeStep *step = maze->stack[maze->stack_ptr];

	maze->stack[maze->stack_ptr] = NULL;

	return step;
}

// create a maze object
Maze *maze_create(int width, int height) {
	Maze *maze = safe_malloc(sizeof(Maze), "maze_create Maze");

	maze->height = height;
	maze->width = width;
	maze->grid = safe_malloc(height * sizeof(int *), "maze_create height");
	maze->stack_ptr = 0;

	// initialize the grid
	for (int i = 0; i < height; i++) {
		maze->grid[i] = safe_malloc(width * sizeof(int *),
		    "maze_create width");
		for (int j = 0; j < width; j++) {
			MazeBlock *block = safe_malloc(sizeof(MazeBlock),
			    "maze_create maze_block");

			block->sides_seen = 0;

			maze->grid[i][j] = block;
		}
	}

	// make room for all possible steps
	maze->stack_max = width * height * 4;
	maze->stack = safe_malloc(maze->stack_max * sizeof(MazeStep *),
	    "maze_create stack");

	// seed the stack with the initial steps
	MazeDirection directions[4] = {
		MAZE_NORTH,
		MAZE_SOUTH,
		MAZE_EAST,
		MAZE_WEST,
	};
	shuffle_array(directions, 4);
	int x = rand() % width;
	int y = rand() % height;
	for (int i = 0; i < 4; i++) {
		maze_step_push(maze, x, y, directions[i]);
	}

	return maze;
}

// single step through the maze - returns a boolean if we are done
bool maze_step(Maze *maze) {
begin:
	// pop a step off the stack
	MazeStep *step = maze_step_pop(maze);

	if (step == NULL) {
		// nothing to do - maze is done
		return true;
	}

	// copy the data from the step and free it - cx/cy == current x /
	// current y
	int cx = step->x;
	int cy = step->y;
	MazeDirection direction = step->direction;
	MazeDirection oppo_direction = opposite_direction(direction);
	free(step);

	// get the current block the step references
	MazeBlock *curBlock = maze->grid[cy][cx];

	if (curBlock->sides_seen & direction) {
		// we already checked this direction - try again
		goto begin;
	}

	// figure out the neighbor block - nx/ny == neighbor x / neighbor y
	int nx = cx + dx(direction, cx);
	int ny = cy + dy(direction, cy);

	// ensure the neighbor is valid / exists
	if (nx < 0 || nx >= maze->width || ny < 0 || ny >= maze->height) {
		// out of bounds - try again
		goto begin;
	}

	// get the new block
	MazeBlock *newBlock = maze->grid[ny][nx];

	if (newBlock->sides_seen > 0) {
		// this block has already been processed - try again
		goto begin;
	}

	// mark the block as seen
	curBlock->sides_seen |= direction;
	newBlock->sides_seen |= oppo_direction;

	// push sides left to be seen
	MazeDirection directions[4] = {
		MAZE_NORTH,
		MAZE_SOUTH,
		MAZE_EAST,
		MAZE_WEST,
	};
	shuffle_array(directions, 4);
	for (int i = 0; i < 4; i++) {
		maze_step_push(maze, nx, ny, directions[i]);
	}

	return false;
}

// destroy the maze
void maze_destroy(Maze *maze) {
	if (maze == NULL) {
		return;
	}

	for (int i = 0; i < maze->height; i++) {
		for (int j = 0; j < maze->width; j++) {
			free(maze->grid[i][j]);
		}
		free(maze->grid[i]);
	}
	free(maze->grid);

	for (int i = 0; i < maze->stack_ptr; i++) {
		free(maze->stack[i]);
	}
	free(maze->stack);

	free(maze);
}
