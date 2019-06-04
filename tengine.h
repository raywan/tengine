#ifndef __TENGINE_H__
#define __TENGINE_H__

#define WIDTH 10
#define HEIGHT 22

typedef struct Board {
  int width;
  int height;
  int *data;
} Board;

typedef enum PieceOrientation {
  PO_ZERO  = 0,
  PO_RIGHT = 1 << 0,
  PO_TWO   = 1 << 1,
  PO_LEFT  = 1 << 2,
} PieceOrientation;

typedef enum PieceType {
  PT_I = 1 << 3,
  PT_O = 1 << 4,
  PT_T = 1 << 5,
  PT_S = 1 << 6,
  PT_Z = 1 << 7,
  PT_J = 1 << 8,
  PT_L = 1 << 9,
} PieceType;

typedef struct Piece {
  PieceType type;
  PieceOrientation orientation;
  // The center point, the point the piece rotates about
  unsigned int x, y;
} Piece;

// Movement
void move_left();
void move_right();
void move_down(); // Increase gravity
void hard_drop();
void rotate_left();
void rotate_right();

// System
void init_system();
Board *get_board();
Piece hold(); // Holds the current piece and swaps to held
Piece get_next_piece();
Piece get_current_piece();
void get_ghost(); // Get the location of the current piece if hard dropped
void update(); // Updates the state of the game
void commit(); // Commits piece to board

// TODO(ray): Scoring

#endif
