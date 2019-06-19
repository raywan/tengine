#ifndef __TENGINE_H__
#define __TENGINE_H__

#include <stdint.h>

#define TENGINE_DEBUG

#define WIDTH 10
#define HEIGHT 20
#define HEIGHT_PADDING 0

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
  int x, y;
} Piece;

typedef struct PieceOffsets {
  int ox_1;
  int ox_2;
  int ox_3;
  int oy_1;
  int oy_2;
  int oy_3;
} PieceOffsets;

typedef struct TState {
  int game_over;

  int score;
  // NOTE(ray): Currently unused
  int combo;

  // When lines cleared reaches current level times 10, increase level by 1
  uint32_t level;
  uint32_t num_lines_cleared;
  RowClearType clear_type;
  RowClearType last_clear_type;

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
  int next_piece_buf_cur_idx;
  PieceType next_piece_buf[5];

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
void move_down();
void hard_drop();
void rotate_left();
void rotate_right();

// System
void te_init_system();

TState *get_state();
Board *get_board();

Piece get_current_piece();
Piece get_next_piece();
PieceOffsets get_piece_offsets(PieceType type);
void hold(); // Holds the current piece and swaps to held
void get_ghost(); // Get the location of the current piece if hard dropped
void commit(); // Commits piece to board

void te_update(int d_frame); // Updates the state of the game

int te_is_game_over();
int te_get_level();
int te_get_score();

// Debugging
void load_board(int data[220]);
Board *get_committed_board();

#ifdef __cplusplus
}
#endif


#endif
