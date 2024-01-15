/**
 * An implementation of the Recursive Backtracking algorithm.
 *
 * https://en.wikipedia.org/wiki/Maze_generation_algorithm#Recursive_implementation
 *
 * The caller can use this with:
 *
 * ```
 * Maze *maze = maze_create(16, 9);
 * maze_step(maze);
 * maze_step(maze);
 * maze_destroy(maze);
 */

/*
 * The 4 cardinal directions
 */
typedef enum MazeDirection {
	MAZE_EAST = 1,
	MAZE_WEST = 2,
	MAZE_NORTH = 4,
	MAZE_SOUTH = 8,
} MazeDirection;

/*
 * A single block in the maze grid
 */
typedef struct MazeBlock {
	int sides_seen;
} MazeBlock;

/*
 * Used internally - a single "step" that is pushed to the stack and popped off
 * as part of `maze_step`.
 */
typedef struct MazeStep {
	int x;
	int y;
	MazeDirection direction;
} MazeStep;

/*
 * The maze object itself.
 *
 * The caller can inspect the "grid" member to see array of all blocks seen
 * after running all steps / iterations.
 */
typedef struct Maze {
	int height;
	int width;
	MazeBlock ***grid;
	MazeStep **stack;
	unsigned int stack_ptr;
	unsigned int stack_max;
} Maze;

// Create a maze object
Maze *maze_create(int width, int height);

// Run a single maze step
bool maze_step(Maze *maze);

// Destroy the maze object and free its memory
void maze_destroy(Maze *maze);
