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
  PT_GHOST = 1 << 10,
} PieceType;

// TODO(ray): Complete this enumeration
typedef enum LineClearType {
  LCT_SINGLE,
  LCT_DOUBLE,
  LCT_TRIPLE,
  LCT_TETRIS,
  LCT_T_SPIN_MINI,
  LCT_T_SPIN,
  LCT_T_SPIN_SINGLE,
  LCT_T_SPIN_MINI_SINGLE,
  LCT_T_SPIN_DOUBLE,
  LCT_T_SPIN_MINI_DOUBLE,
  LCT_T_SPIN_TRIPLE,
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

  // When lines cleared reaches current level times 10, increase level by 1
  uint32_t level;
  uint32_t num_lines_cleared;
  RowClearType clear_type;
  RowClearType last_clear_type;

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

  int cur_bag_idx;
  PieceType cur_piece_idx_in_bag;

  // Frames before locking (committing) the piece if colliding
  int lock_delay_fr;
  int lock_delay_fr_counter;
  // Frames before moving the current piece down naturally
  int gravity_fr; // cells per frame
  int gravity_fr_counter;
  // DAS - frames before auto shifting from holding left/right down
  int delay_auto_shift_fr;
  int delay_auto_shift_fr_counter;

} TState;

#ifdef __cplusplus
extern "C" {
#endif

// Movement
void move_left();
void move_right();
void move_down(); // Increase gravity
void hard_drop();
void rotate_left();
void rotate_right();

// System
void te_init_system();

TState *get_state();
Board *get_board();
Board *get_committed_board();

Piece get_current_piece();
Piece get_next_piece();
void hold(); // Holds the current piece and swaps to held
void get_ghost(); // Get the location of the current piece if hard dropped
void commit(); // Commits piece to board

void te_update(int dt_frame); // Updates the state of the game

// Debug

void load_board(int data[220]);

#ifdef __cplusplus
}
#endif


#endif
