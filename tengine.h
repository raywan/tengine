#ifndef __TENGINE_H__
#define __TENGINE_H__

#include <stdint.h>

#define TENGINE_DEBUG

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

// TODO(ray): Complete this enumeration
typedef enum RowClearType {
  T_SPIN,
  TETRIS,
  // ...
} RowClearType;

typedef struct Piece {
  PieceType type;
  PieceOrientation orientation;
  // The center point, the point the piece rotates about
  unsigned int x, y;
} Piece;

typedef struct TState {
  int score;
  int combo;

  // TODO(ray): Maybe unify this data instead of keeping it in the board
  uint32_t board_width;
  uint32_t board_height;

  Piece next_piece_buf[5];
  Piece cur_piece;
  Piece ghost_piece;
  Piece held_piece;
  char has_swapped;
  char is_initial_swap;

  // Board used to render. Combines data from committed_board, cur_piece and ghost_piece
  // NOTE(ray): Semantically, we're treating row 22 in the array to be the bottom
  // of the game board. Might be better to change this later.
  Board board;
  // Holds all committed pieces on the board
  Board committed_board;
} TState;

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
Board *get_committed_board();

Piece get_current_piece();
Piece get_next_piece();
void hold(); // Holds the current piece and swaps to held
void get_ghost(); // Get the location of the current piece if hard dropped
void commit(); // Commits piece to board

void update(); // Updates the state of the game


#endif
