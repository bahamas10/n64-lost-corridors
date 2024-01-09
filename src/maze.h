typedef enum MazeDirection {
	MAZE_EAST = 1,
	MAZE_WEST = 2,
	MAZE_NORTH = 4,
	MAZE_SOUTH = 8,
} MazeDirection;

typedef struct MazeBlock {
	int sides_seen;
} MazeBlock;

typedef struct MazeStep {
	int x;
	int y;
	MazeDirection direction;
} MazeStep;

typedef struct Maze {
	int height;
	int width;
	MazeBlock ***grid;
	MazeStep **stack;
	unsigned int stack_ptr;
	unsigned int stack_max;
} Maze;

Maze *maze_create(int width, int height);
void maze_step(Maze *maze);
void maze_destroy(Maze *maze);
