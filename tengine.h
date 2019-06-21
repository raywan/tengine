#ifndef __TENGINE_H__
#define __TENGINE_H__

#include <stdint.h>

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
} LineClearType;

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
  LineClearType clear_type;
  LineClearType last_clear_type;

  Piece cur_piece;
  Piece ghost_piece;
  Piece held_piece;
  char has_swapped;
  char is_initial_swap;

  // Board used to render. Combines data from committed_board, cur_piece and ghost_piece
  // NOTE(ray): Semantically, we're treating row 20 in the array to be the bottom
  // of the game board. Might be better to change this later.
  Board board;
  // Holds all committed pieces on the board
  Board committed_board;

  int cur_bag_idx;
  PieceType cur_piece_idx_in_bag;
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
void te_move_left();
void te_move_right();
void te_move_down();
void te_hard_drop();
void te_rotate_left();
void te_rotate_right();

// Initialize the system
void te_init_system();

// Free resources allocated
void te_destroy_system();

// Get the game state
TState *te_get_state();

// Get the board, used for rendering
Board *te_get_board();

// Get the value of the board at index i and j
int te_get_board_xy(Board *b, int x, int y);

// Get the current piece
Piece te_get_current_piece();

// Get the buffer to the next piece buffer
// Used for rendering the next piece list
// Returns a pointer to the start of the buffer and length in out_buf_length
PieceType *te_get_next_piece_buf(int *out_buf_len);

// Get the offsets for a piece, that can be used to render pieces individually
PieceOffsets te_get_piece_offsets(PieceType type, PieceOrientation orientation);

// Holds the current piece and swaps to held
void te_hold();

// Get the location of the current piece if hard dropped
void te_get_ghost();

// Commits piece to board
void te_commit();

// Updates the state of the game
void te_update(int d_frame);

// Did we top out?
int te_is_game_over();

// Get the current level
int te_get_level();

// Get the current score
int te_get_score();

// The following are for debugging, you probably won't use these
void te_load_board(int data[220]);
Board *te_get_committed_board();
Piece te_get_next_piece();

#ifdef __cplusplus
}
#endif


#endif
