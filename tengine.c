#include "tengine.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if !defined(internal)
#define internal static
#endif
#define local_persist static
#define global static

#if !defined(MIN)
#define MIN(a,b) ((a) > (b) ? (b) : (a))
#endif

#if !defined(MAX)
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

// NOTE(ray): Three x-y offsets are encoded using 24-bits (8 bits for each)
// 00000000 00000000 00000000
// For each 8 bits, the upper 4 bits are the x value and the lower 4 are the y value.
#define OFFSET_1_HEX(raw_offsets) (raw_offsets & 0xFF)
#define OFFSET_2_HEX(raw_offsets) ((raw_offsets & 0xFF00) >> 8)
#define OFFSET_3_HEX(raw_offsets) ((raw_offsets & 0xFF0000) >> 16)
#define OFFSET_X_MASK 0xF0
#define OFFSET_Y_MASK 0xF
#define OFFSET_1_X_CODE(raw_offsets) ((OFFSET_1_HEX(raw_offsets) & OFFSET_X_MASK) >> 4)
#define OFFSET_2_X_CODE(raw_offsets) ((OFFSET_2_HEX(raw_offsets) & OFFSET_X_MASK) >> 4)
#define OFFSET_3_X_CODE(raw_offsets) ((OFFSET_3_HEX(raw_offsets) & OFFSET_X_MASK) >> 4)
#define OFFSET_1_Y_CODE(raw_offsets) (OFFSET_1_HEX(raw_offsets) & OFFSET_Y_MASK)
#define OFFSET_2_Y_CODE(raw_offsets) (OFFSET_2_HEX(raw_offsets) & OFFSET_Y_MASK)
#define OFFSET_3_Y_CODE(raw_offsets) (OFFSET_3_HEX(raw_offsets) & OFFSET_Y_MASK)

#define DEFAULT_SPAWN_X 4
#define DEFAULT_SPAWN_Y 0

internal TState t_state;

internal PieceType piece_permutations[5040][7];

internal int kick_test_table[40][2] = {
  {0,0}, {-1,0}, {-1,1},  {0,-2}, {-1,-2}, // 0 -> R
  {0,0}, {1,0},  {1,-1},  {0,2},  {1,2},   // R -> 0
  {0,0}, {1,0},  {1,-1},  {0,2},  {1,2},   // R -> 2
  {0,0}, {-1,0}, {-1,1},  {0,-2}, {-1,-2}, // 2 -> R

  {0,0}, {1,0},  {1,1},   {0,-2}, {1,-2},  // 2 -> L
  {0,0}, {-1,0}, {-1,-1}, {0,2},  {-1,2},  // L -> 2
  {0,0}, {-1,0}, {-1,-1}, {0,2},  {-1,2},  // L -> 0
  {0,0}, {1,0},  {1,1},   {0,-2}, {1,-2}   // 0 -> L
};

#if 0
// True kick test table for I
internal int i_kick_test_table[40][2] = {
  {0,0}, {-2,0}, {1,0},   {-2,-1}, {1,2},   // 0 -> R
  {0,0}, {2,0},  {-1,0},  {2,1},   {-1,-2}, // R -> 0
  {0,0}, {-1,0}, {2,0},   {-1,2},  {2,-1},  // R -> 2
  {0,0}, {1,0},  {-2,0},  {1,-2},  {-2,1},  // 2 -> R

  {0,0}, {2,0},  {-1,1},  {2,1},   {-1,-2}, // 2 -> L
  {0,0}, {-2,0}, {1,-1},  {-2,-1}, {1,2},   // L -> 2
  {0,0}, {1,0},  {-2,-1}, {1,-2},  {-2,1},  // L -> 0
  {0,0}, {-1,0}, {2,0},   {-1,2},  {2,-1}   // 0 -> L
};
#else
// NOTE(ray): Tetris I-pieces aren't centered around a block.
// This is not the case for this implementation. This new table compensates for that.
// Use the above if representation every changes.
internal int i_kick_test_table[40][2] = {
  {1,0},  {-1,0},  {2,0},  {-1,-1}, {2,2},    // 0 -> R
  {-1,0}, {0,1},   {-2,0}, {1,1},   {-2,-2},  // R -> 0
  {0,-1}, {-1,-1}, {2,-1}, {-1,1},  {2,-2},   // R -> 2
  {0,1},  {1,1},   {-2,1}, {1,-1},  {-2,2},   // 2 -> R

  {-1,0}, {1,0},   {-2,0},  {1,1},   {-2,-2}, // 2 -> L
  {1,0},  {-1,0},  {2,0},   {-1,-1}, {2,2},   // L -> 2
  {0,1},  {1,1},   {-2,1},  {1,-1},  {-2,2},  // L -> 0
  {0,-1}, {-1,-1}, {2,-1},  {-1,1},  {2,-2}   // 0 -> L
};
#endif

// NOTE(ray): Table follows 2009 guidelines
// http://harddrop.com/wiki/Scoring
internal int score_table[11] = { 100, 300, 500, 800, 100, 400, 200, 800, 1200, 1200, 1600 };

// __INTERNAL

// Returns bit representation of the offsets
internal int get_raw_offsets(PieceType type, PieceOrientation orientation) {
  switch (type | orientation) {
    case PT_I | PO_ZERO:
      // puts("PT_I | PO_ZERO");
      return 0x201030;
    case PT_I | PO_RIGHT:
      // puts("PT_I | PO_RIGHT");
      return 0x20103;
    case PT_I | PO_TWO:
      // puts("PT_I | PO_TWO");
      return 0x102040;
    case PT_I | PO_LEFT:
      // puts("PT_I | PO_LEFT");
      return 0x20401;

    case PT_O | PO_ZERO:
    case PT_O | PO_RIGHT:
    case PT_O | PO_TWO:
    case PT_O | PO_LEFT:
      return 0x100111;

    case PT_T | PO_ZERO:
      // puts("PT_T | PO_ZERO");
      return 0x201002;
    case PT_T | PO_RIGHT:
      // puts("PT_T | PO_RIGHT");
      return 0x11002;
    case PT_T | PO_TWO:
      // puts("PT_T | PO_TWO");
      return 0x12010;
    case PT_T | PO_LEFT:
      // puts("PT_T | PO_LEFT");
      return 0x12002;

    case PT_S | PO_ZERO:
      // puts("PT_S | PO_ZERO");
      return 0x200212;
    case PT_S | PO_RIGHT:
      // puts("PT_S | PO_RIGHT");
      return 0x111002;
    case PT_S | PO_TWO:
      // puts("PT_S | PO_TWO");
      return 0x210110;
    case PT_S | PO_LEFT:
      // puts("PT_S | PO_LEFT");
      return 0x12022;

    case PT_Z | PO_ZERO:
      // puts("PT_Z | PO_ZERO");
      return 0x102202;
    case PT_Z | PO_RIGHT:
      // puts("PT_Z | PO_RIGHT");
      return 0x11012;
    case PT_Z | PO_TWO:
      // puts("PT_Z | PO_TWO");
      return 0x11120;
    case PT_Z | PO_LEFT:
      // puts("PT_Z | PO_LEFT");
      return 0x212002;

    case PT_J | PO_ZERO:
      // puts("PT_J | PO_ZERO");
      return 0x201022;
    case PT_J | PO_RIGHT:
      // puts("PT_J | PO_RIGHT");
      return 0x10212;
    case PT_J | PO_TWO:
      // puts("PT_J | PO_TWO");
      return 0x112010;
    case PT_J | PO_LEFT:
      // puts("PT_J | PO_LEFT");
      return 0x210102;

    case PT_L | PO_ZERO:
      // puts("PT_L | PO_ZERO");
      return 0x201012;
    case PT_L | PO_RIGHT:
      // puts("PT_L | PO_RIGHT");
      return 0x11102;
    case PT_L | PO_TWO:
      // puts("PT_L | PO_TWO");
      return 0x212010;
    case PT_L | PO_LEFT:
      // puts("PT_L | PO_LEFT");
      return 0x12202;
    default:
      return -1;
  }
}

internal inline int offset_code_get_int_value(int code) {
  switch (code) {
    case 0x1:
      return 1;
    case 0x2:
      return -1;
    case 0x3:
      return 2;
    case 0x4:
      return -2;
    default:
      return 0;
  }
}

internal inline int get_min_x_offset() {
  int offsets = get_raw_offsets(t_state.cur_piece.type, t_state.cur_piece.orientation);
  int o_1 = offset_code_get_int_value(OFFSET_1_HEX(offsets) & OFFSET_X_MASK);
  int o_2 = offset_code_get_int_value(OFFSET_2_HEX(offsets) & OFFSET_X_MASK);
  int o_3 = offset_code_get_int_value(OFFSET_3_HEX(offsets) & OFFSET_X_MASK);
  return MIN(o_1, MIN(o_2, o_3));
}

internal inline int get_max_x_offset() {
  int offsets = get_raw_offsets(t_state.cur_piece.type, t_state.cur_piece.orientation);
  int o_1 = offset_code_get_int_value(OFFSET_1_HEX(offsets) & OFFSET_X_MASK);
  int o_2 = offset_code_get_int_value(OFFSET_2_HEX(offsets) & OFFSET_X_MASK);
  int o_3 = offset_code_get_int_value(OFFSET_3_HEX(offsets) & OFFSET_X_MASK);
  return MAX(o_1, MAX(o_2, o_3));
}

internal inline int get_min_y_offset() {
  int offsets = get_raw_offsets(t_state.cur_piece.type, t_state.cur_piece.orientation);
  int o_1 = OFFSET_1_HEX(offsets) & OFFSET_Y_MASK;
  int o_2 = OFFSET_2_HEX(offsets) & OFFSET_Y_MASK;
  int o_3 = OFFSET_3_HEX(offsets) & OFFSET_Y_MASK;
  return MIN(o_1, MIN(o_2, o_3));
}

internal inline int get_max_y_offset() {
  int offsets = get_raw_offsets(t_state.cur_piece.type, t_state.cur_piece.orientation);
  int o_1 = offset_code_get_int_value(OFFSET_1_HEX(offsets) & OFFSET_Y_MASK);
  int o_2 = offset_code_get_int_value(OFFSET_2_HEX(offsets) & OFFSET_Y_MASK);
  int o_3 = offset_code_get_int_value(OFFSET_3_HEX(offsets) & OFFSET_Y_MASK);
  return MAX(o_1, MAX(o_2, o_3));
}

internal inline int is_board_xy_filled(int x, int y) {
  // Implicitly define that trying to go out of bounds is hitting a filled section
  // printf("is_board_xy_filled(x = %d, y = %d)\n", x, y);
  if (x < 0 || x >= t_state.committed_board.width) {
     // printf("FAILED WIDTH CHECK: x = %d, width: %d\n", x, t_state.committed_board.width);
    return 1;
  // } else if (y < 0 || y >= t_state.committed_board.height) {
  } else if (y >= t_state.committed_board.height) {
     // puts("FAILED HEIGHT CHECK");
    return 1;
  } else if (y < 0) {
    // NOTE(ray): Don't consider topping out to be "colliding". Handle this differently.
    return 0;
  }
  return t_state.committed_board.data[t_state.committed_board.width * y + x] != -1;
}

internal inline int will_piece_collide(PieceType type, PieceOrientation orientation, int potential_x, int potential_y) {
  int offsets = get_raw_offsets(type, orientation);
  int ox_1 = offset_code_get_int_value(OFFSET_1_X_CODE(offsets));
  int ox_2 = offset_code_get_int_value(OFFSET_2_X_CODE(offsets));
  int ox_3 = offset_code_get_int_value(OFFSET_3_X_CODE(offsets));
  int oy_1 = offset_code_get_int_value(OFFSET_1_Y_CODE(offsets));
  int oy_2 = offset_code_get_int_value(OFFSET_2_Y_CODE(offsets));
  int oy_3 = offset_code_get_int_value(OFFSET_3_Y_CODE(offsets));

  if (!is_board_xy_filled(potential_x, potential_y) &&
      !is_board_xy_filled(potential_x + ox_1, potential_y + oy_1) &&
      !is_board_xy_filled(potential_x + ox_2, potential_y + oy_2) &&
      !is_board_xy_filled(potential_x + ox_3, potential_y + oy_3)) {
    return 0;
  }
  return 1;
}

internal void set_board_data(Board *b, int x, int y, int i) {
  b->data[b->width * y + x] = i;
}

internal void set_piece(Board *b, Piece p, int i) {
  int raw_offsets = get_raw_offsets(p.type, p.orientation);
  int ox_1 = offset_code_get_int_value(OFFSET_1_X_CODE(raw_offsets));
  int ox_2 = offset_code_get_int_value(OFFSET_2_X_CODE(raw_offsets));
  int ox_3 = offset_code_get_int_value(OFFSET_3_X_CODE(raw_offsets));
  int oy_1 = offset_code_get_int_value(OFFSET_1_Y_CODE(raw_offsets));
  int oy_2 = offset_code_get_int_value(OFFSET_2_Y_CODE(raw_offsets));
  int oy_3 = offset_code_get_int_value(OFFSET_3_Y_CODE(raw_offsets));
  set_board_data(b, p.x, p.y, i);
  set_board_data(b, p.x + ox_1, p.y + oy_1, i);
  set_board_data(b, p.x + ox_2, p.y + oy_2, i);
  set_board_data(b, p.x + ox_3, p.y + oy_3, i);
}

internal void generate_permutations(PieceType arr[7], int k, int *perm_idx) {
  if (k == 1) {
    for (int i = 0; i < 7; i++) {
      piece_permutations[*perm_idx][i] = arr[i];
    }
  } else {
    generate_permutations(arr, k - 1, perm_idx);
    for (int i = 0; i < k - 1; i++) {
      if (k % 2 == 0) {
        PieceType tmp = arr[i];
        arr[i] = arr[k-1];
        arr[k - 1] = tmp;
      } else {
        PieceType tmp = arr[0];
        arr[0] = arr[k-1];
        arr[k - 1] = tmp;
      }
      (*perm_idx)++;
      generate_permutations(arr, k - 1, perm_idx);
    }
  }
}

// __SYSTEM

void te_init_system() {
  srand(420);

  // Allocate memory for board data
  t_state.board.width = WIDTH;
  t_state.board.height = HEIGHT;
  t_state.board.data = (int *) malloc(sizeof(int) * WIDTH * HEIGHT);

  t_state.committed_board.width = WIDTH;
  t_state.committed_board.height = HEIGHT;
  t_state.committed_board.data = (int *) malloc(sizeof(int) * WIDTH * HEIGHT);

  for (int i = 0; i < t_state.board.height; i++) {
    for (int j = 0; j < t_state.board.width; j++) {
      t_state.board.data[t_state.board.width * i + j] = -1;
      t_state.committed_board.data[t_state.board.width * i + j] = -1;
    }
  }

  // Generate permuations
  int perm_idx = 0;
  PieceType arr[7] = { PT_I, PT_O, PT_T, PT_S, PT_Z, PT_J, PT_L };
  generate_permutations(arr, 7, &perm_idx);
  // Initially pick a random bag
  t_state.cur_bag_idx = (int) (rand() % (5040-1));
  t_state.cur_piece_idx_in_bag = 0;

  t_state.game_over = 0;
  t_state.score = 0;
  t_state.combo = -1;
  t_state.level = 1;
  t_state.num_lines_cleared = 0;

  t_state.has_swapped = 0;
  t_state.is_initial_swap = 1;

  t_state.cur_piece = get_next_piece();

  t_state.lock_delay_fr = 30;
  t_state.lock_delay_fr_counter = 0;
  t_state.gravity_fr = 60;
  t_state.gravity_fr_counter = 0;
}

TState *get_state() {
  return &t_state;
}

Board *get_board() {
  return &t_state.board;
}

Board *get_committed_board() {
  return &t_state.committed_board;
}

// Holds the current piece and swaps to held
void hold() {
  if (t_state.is_initial_swap) {
    t_state.held_piece = t_state.cur_piece;
    t_state.cur_piece = get_next_piece();
    t_state.has_swapped = 1;
    t_state.is_initial_swap = 0;
  } else if (!t_state.has_swapped) {
    Piece temp = t_state.held_piece;
    t_state.held_piece = t_state.cur_piece;
    t_state.cur_piece = temp;
    t_state.cur_piece.x = DEFAULT_SPAWN_X;
    t_state.cur_piece.y = DEFAULT_SPAWN_Y;
    t_state.has_swapped = 1;
  }
}

Piece get_next_piece() {
  if (t_state.cur_piece_idx_in_bag > 6) {
    // Reset idx
    t_state.cur_piece_idx_in_bag = 0;
    // Generate a new bag
    t_state.cur_bag_idx = (int) (rand() % (5040-1));
  }

  // Get the next piece in the bag
  Piece result;
  result.type = piece_permutations[t_state.cur_bag_idx][t_state.cur_piece_idx_in_bag];
  // result.type = PT_T;
  result.orientation = PO_ZERO;
  result.x = DEFAULT_SPAWN_X;
  result.y = DEFAULT_SPAWN_Y;
  ++t_state.cur_piece_idx_in_bag;
  // Reset lock delay timer
  t_state.lock_delay_fr_counter = 0;
  // Check if we're spawning on any filled pieces
  if (will_piece_collide(result.type, result.orientation, result.x, result.y)) {
    t_state.game_over = 1;
  }
  return result;
}

Piece get_current_piece() {
  Piece result = t_state.cur_piece;
  return result;
}

// Get the location of the current piece if hard dropped
void get_ghost() {
  int offsets = get_raw_offsets(t_state.cur_piece.type, t_state.cur_piece.orientation);
  int ox_1 = offset_code_get_int_value(OFFSET_1_X_CODE(offsets));
  int ox_2 = offset_code_get_int_value(OFFSET_2_X_CODE(offsets));
  int ox_3 = offset_code_get_int_value(OFFSET_3_X_CODE(offsets));
  int oy_1 = offset_code_get_int_value(OFFSET_1_Y_CODE(offsets));
  int oy_2 = offset_code_get_int_value(OFFSET_2_Y_CODE(offsets));
  int oy_3 = offset_code_get_int_value(OFFSET_3_Y_CODE(offsets));

  int ghost_y = 0;
  // TODO(ray): Check starting from the bottom instead. May lead to earlier break
  // Reverse in logic means keep moving up until there are no collisions.
  for (int i = 0; i < HEIGHT - t_state.cur_piece.y; i++) {
    if (will_piece_collide(t_state.cur_piece.type, t_state.cur_piece.orientation, t_state.cur_piece.x, t_state.cur_piece.y + i)) {
      break;
    }
    ghost_y = t_state.cur_piece.y + i;
  }
  t_state.ghost_piece.type = t_state.cur_piece.type;
  t_state.ghost_piece.orientation = t_state.cur_piece.orientation;
  t_state.ghost_piece.x = t_state.cur_piece.x;
  t_state.ghost_piece.y = ghost_y;
}

// Updates the state of the game
void te_update(int dt_frame) {

  if (t_state.game_over) return;

  // Clear the board in preparation for next render
  for (int i = 0; i < t_state.board.height; i++) {
    for (int j = 0; j < t_state.board.width; j++) {
      t_state.board.data[t_state.board.width * i + j] = -1;
    }
  }

  // Try to move down
  if (will_piece_collide(t_state.cur_piece.type, t_state.cur_piece.orientation, t_state.cur_piece.x, t_state.cur_piece.y + 1)) {
    // Check lock delay
    if (t_state.lock_delay_fr_counter >= t_state.lock_delay_fr) {
      // Reset counter
      t_state.lock_delay_fr_counter = 0;
      // Lock
      hard_drop();
    } else {
      ++t_state.lock_delay_fr_counter;
    }
  }
  else {
    if (t_state.gravity_fr_counter >= t_state.gravity_fr) {
      t_state.gravity_fr_counter = 0;
      move_down();
    } else {
      ++t_state.gravity_fr_counter;
    }
  }

  get_ghost();

  // TODO(ray): Handle soft drop
  // If we've been colliding for 30 frames, commit
#if 0
  int raw_offsets = get_raw_offsets(t_state.cur_piece.type, t_state.cur_piece.orientation);
  int ox_1 = offset_code_get_int_value(OFFSET_1_X_CODE(raw_offsets));
  int ox_2 = offset_code_get_int_value(OFFSET_2_X_CODE(raw_offsets));
  int ox_3 = offset_code_get_int_value(OFFSET_3_X_CODE(raw_offsets));
  int oy_1 = offset_code_get_int_value(OFFSET_1_Y_CODE(raw_offsets));
  int oy_2 = offset_code_get_int_value(OFFSET_2_Y_CODE(raw_offsets));
  int oy_3 = offset_code_get_int_value(OFFSET_3_Y_CODE(raw_offsets));
  printf("t_state.cur_piece.x = %d\ncur_piece.y = %d\n", t_state.cur_piece.x, t_state.cur_piece.y);
  printf("ox_1: %d, ox_2: %d, ox_3: %d\noy_1: %d, oy_2: %d, oy_3: %d\n", ox_1, ox_2, ox_3, oy_1, oy_2, oy_3);
  printf("ghost_piece.x = %d\nghost_piece.y = %d\n", t_state.ghost_piece.x, t_state.ghost_piece.y);
  printf("held_piece type %d\n", t_state.held_piece.type);
  printf("num_lines_cleared: %d\n", t_state.num_lines_cleared);
#endif

  // NOTE(ray): The following operations are for rendering the board
  // Copy the commited_board to the board
  memcpy(t_state.board.data, t_state.committed_board.data, sizeof(int) * WIDTH * HEIGHT);
  // Set the ghost
  set_piece(&t_state.board, t_state.ghost_piece, PT_GHOST);
  // Set the piece
  set_piece(&t_state.board, t_state.cur_piece, t_state.cur_piece.type);
}

// Commits piece to board
void commit() {

  // Check for partial top-out 
  int raw_offsets = get_raw_offsets(t_state.cur_piece.type, t_state.cur_piece.orientation);
  int oy_1 = offset_code_get_int_value(OFFSET_1_Y_CODE(raw_offsets));
  int oy_2 = offset_code_get_int_value(OFFSET_2_Y_CODE(raw_offsets));
  int oy_3 = offset_code_get_int_value(OFFSET_3_Y_CODE(raw_offsets));
  if ((t_state.cur_piece.y < 0) || (t_state.cur_piece.y + oy_1 < 0) || (t_state.cur_piece.y + oy_2 < 0) || (t_state.cur_piece.y + oy_3 < 0)) {
    t_state.game_over = 1;
  }

  // Write the piece to the committed board
  set_piece(&t_state.committed_board, t_state.cur_piece, t_state.cur_piece.type);

  // 1. Check bottom up if there are any filled rows
  // 2. Mark filled rows
  // 3. Compact from the top down
  uint8_t num_rows_filled = 0;
  // In normal Tetris, At most 4 rows can be cleared at once
  uint8_t filled_row_markers[4] = { 0, 0, 0, 0 };
  for (int i = t_state.committed_board.height-1; i >= 0; i--) {
    char is_row_filled = 1;
    // Check if the row has been filled
    for (int j = 0; j < t_state.committed_board.width; j++) {
      is_row_filled &= t_state.committed_board.data[t_state.committed_board.width * i + j] != -1;
    }
    if (is_row_filled) {
      // Clear the row from the board
      for (int j = 0; j < t_state.committed_board.width; j++) {
        t_state.committed_board.data[t_state.committed_board.width * i + j] = -1;
      }
      // Mark the row for later
      filled_row_markers[num_rows_filled++] = i;
    }
  }

  if (num_rows_filled > 0) {
    t_state.num_lines_cleared += num_rows_filled;
    printf("num_lines_cleared: %d\n", t_state.num_lines_cleared);
    // Compact starting from the top down
    for (int i = num_rows_filled-1; i >= 0; i--) {
      // Start from the cleared row and start moving stuff down
      for (int j = filled_row_markers[i]; j >= 0; j--) {
        for (int k = 0; k < t_state.board.width; k++) {
          if (j - 1 >= 0) {
            t_state.committed_board.data[t_state.committed_board.width * j + k] = t_state.committed_board.data[t_state.committed_board.width * (j - 1) + k];
          } else {
            t_state.committed_board.data[t_state.committed_board.width * j + k] = -1;
          }
        }
      }
    }

    t_state.last_clear_type = t_state.clear_type;
    if (num_rows_filled == 1) {
      t_state.clear_type = LCT_SINGLE;
    } else if (num_rows_filled == 2) {
      t_state.clear_type = LCT_DOUBLE;
    } else if (num_rows_filled == 3) {
      t_state.clear_type = LCT_TRIPLE;
    } else if (num_rows_filled == 4) {
      t_state.clear_type = LCT_TETRIS;
    }

    // Compute the score
    t_state.score += score_table[t_state.clear_type] * t_state.level;

    printf("score: %d\n", t_state.score);
  }

  // Increase the level if applicable
  if (t_state.num_lines_cleared == t_state.level * 10) {
    ++t_state.level;
    printf("level: %d\n", t_state.level);
  }

  // Grab the next piece
  t_state.cur_piece = get_next_piece();
  // Reset swap state
  t_state.has_swapped = 0;
}

// __MOVEMENT

void move_left() {
  // Check all negative x offsets
  int offsets = get_raw_offsets(t_state.cur_piece.type, t_state.cur_piece.orientation);
  int ox_1 = offset_code_get_int_value(OFFSET_1_X_CODE(offsets));
  int ox_2 = offset_code_get_int_value(OFFSET_2_X_CODE(offsets));
  int ox_3 = offset_code_get_int_value(OFFSET_3_X_CODE(offsets));
  int oy_1 = offset_code_get_int_value(OFFSET_1_Y_CODE(offsets));
  int oy_2 = offset_code_get_int_value(OFFSET_2_Y_CODE(offsets));
  int oy_3 = offset_code_get_int_value(OFFSET_3_Y_CODE(offsets));
  // There are two cases to consider.
  // 1. We're trying to move past the edge of the board
  // 2. We're moving into a filled in section of the board horizontally

  // Moving into another filled block on the board, stop the move.
  // Check all negative x offsets. If any of the negative offset blocks are
  int potential_x = t_state.cur_piece.x - 1;

  if (is_board_xy_filled(potential_x, t_state.cur_piece.y)) {
    return;
  }

  if (ox_1 <= 0 && is_board_xy_filled(potential_x + ox_1, t_state.cur_piece.y + oy_1)) {
    return;
  }

  if (ox_2 <= 0 && is_board_xy_filled(potential_x + ox_2, t_state.cur_piece.y + oy_2)) {
    return;
  }

  if (ox_3 <= 0 && is_board_xy_filled(potential_x + ox_3, t_state.cur_piece.y + oy_3)) {
    return;
  }

  t_state.cur_piece.x = potential_x;
}

void move_right() {
  // Check all positive x offsets
  int offsets = get_raw_offsets(t_state.cur_piece.type, t_state.cur_piece.orientation);
  int ox_1 = offset_code_get_int_value(OFFSET_1_X_CODE(offsets));
  int ox_2 = offset_code_get_int_value(OFFSET_2_X_CODE(offsets));
  int ox_3 = offset_code_get_int_value(OFFSET_3_X_CODE(offsets));
  int oy_1 = offset_code_get_int_value(OFFSET_1_Y_CODE(offsets));
  int oy_2 = offset_code_get_int_value(OFFSET_2_Y_CODE(offsets));
  int oy_3 = offset_code_get_int_value(OFFSET_3_Y_CODE(offsets));
  // There are two cases to consider.
  // 1. We're trying to move past the edge of the board
  // 2. We're moving into a filled in section of the board horizontally

  // Moving into another filled block on the board, stop the move.
  // Check all negative x offsets. If any of the negative offset blocks are
  int potential_x = t_state.cur_piece.x + 1;

  if (is_board_xy_filled(potential_x, t_state.cur_piece.y)) {
    return;
  }

  if (ox_1 >= 0 && is_board_xy_filled(potential_x + ox_1, t_state.cur_piece.y + oy_1)) {
    return;
  }

  if (ox_2 >= 0 && is_board_xy_filled(potential_x + ox_2, t_state.cur_piece.y + oy_2)) {
    return;
  }

  if (ox_3 >= 0 && is_board_xy_filled(potential_x + ox_3, t_state.cur_piece.y + oy_3)) {
    return;
  }

  t_state.cur_piece.x = potential_x;
}

// Increase gravity
void move_down() {
  int offsets = get_raw_offsets(t_state.cur_piece.type, t_state.cur_piece.orientation);
  int ox_1 = offset_code_get_int_value(OFFSET_1_X_CODE(offsets));
  int ox_2 = offset_code_get_int_value(OFFSET_2_X_CODE(offsets));
  int ox_3 = offset_code_get_int_value(OFFSET_3_X_CODE(offsets));
  int oy_1 = offset_code_get_int_value(OFFSET_1_Y_CODE(offsets));
  int oy_2 = offset_code_get_int_value(OFFSET_2_Y_CODE(offsets));
  int oy_3 = offset_code_get_int_value(OFFSET_3_Y_CODE(offsets));

  int potential_y = t_state.cur_piece.y + 1;
  if (is_board_xy_filled(t_state.cur_piece.x, potential_y)) {
    return;
  }

  if (oy_1 >= 0 && is_board_xy_filled(t_state.cur_piece.x + ox_1, potential_y + oy_1)) {
    return;
  }

  if (oy_2 >= 0 && is_board_xy_filled(t_state.cur_piece.x + ox_2, potential_y + oy_2)) {
    return;
  }

  if (oy_3 >= 0 && is_board_xy_filled(t_state.cur_piece.x + ox_3, potential_y + oy_3)) {
    return;
  }

  t_state.cur_piece.y = potential_y;
}

// Sets current position to ghost position
void hard_drop() {
  // Set the current piece's position to be the ghost position
  t_state.cur_piece.x = t_state.ghost_piece.x;
  t_state.cur_piece.y = t_state.ghost_piece.y;
  // Commit the piece to the board
  commit();
}

// Try to rotate, mutates the position
internal void kick_test(PieceOrientation from_ori, PieceOrientation to_ori) {

  int kick_table_vidx = -1;
  if (from_ori == PO_ZERO && to_ori == PO_RIGHT) {
    kick_table_vidx = 0;
  } else if (from_ori == PO_RIGHT && to_ori == PO_ZERO) {
    kick_table_vidx = 1;
  } else if (from_ori == PO_RIGHT && to_ori == PO_TWO) {
    kick_table_vidx = 2;
  } else if (from_ori == PO_TWO && to_ori == PO_RIGHT) {
    kick_table_vidx = 3;
  } else if (from_ori == PO_TWO && to_ori == PO_LEFT) {
    kick_table_vidx = 4;
  } else if (from_ori == PO_LEFT && to_ori == PO_TWO) {
    kick_table_vidx = 5;
  } else if (from_ori == PO_LEFT && to_ori == PO_ZERO) {
    kick_table_vidx = 6;
  } else if (from_ori == PO_ZERO && to_ori == PO_LEFT) {
    kick_table_vidx = 7;
  }

  for (int i = 0; i < 5; i++) {
    int potential_x;
    int potential_y;
    // I pieces have unique kick tests
    if (t_state.cur_piece.type == PT_I) {
      potential_x = t_state.cur_piece.x + i_kick_test_table[5 * kick_table_vidx + i][0];
      potential_y = t_state.cur_piece.y - i_kick_test_table[5 * kick_table_vidx + i][1];
    } else {
      potential_x = t_state.cur_piece.x + kick_test_table[5 * kick_table_vidx + i][0];
      potential_y = t_state.cur_piece.y - kick_test_table[5 * kick_table_vidx + i][1];
    }

    if (!will_piece_collide(t_state.cur_piece.type, to_ori, potential_x, potential_y)) {
      // printf("SUCCESS KICK v: %d i: %d\n", kick_table_vidx, i);
      t_state.cur_piece.x = potential_x;
      t_state.cur_piece.y = potential_y;
      t_state.cur_piece.orientation = to_ori;
      return;
    }
  }
}

void rotate_left() {
  PieceOrientation to_ori;
  switch (t_state.cur_piece.orientation) {
    case PO_ZERO:
      to_ori = PO_LEFT;
      break;
    case PO_RIGHT:
      to_ori = PO_ZERO;
      break;
    case PO_TWO:
      to_ori = PO_RIGHT;
      break;
    case PO_LEFT:
      to_ori = PO_TWO;
      break;
    default:
      break;
  }
  kick_test(t_state.cur_piece.orientation, to_ori);
}

void rotate_right() {
  PieceOrientation to_ori;
  switch (t_state.cur_piece.orientation) {
    case PO_ZERO:
      to_ori = PO_RIGHT;
      break;
    case PO_RIGHT:
      to_ori = PO_TWO;
      break;
    case PO_TWO:
      to_ori = PO_LEFT;
      break;
    case PO_LEFT:
      to_ori = PO_ZERO;
    default:
      break;
  }
  kick_test(t_state.cur_piece.orientation, to_ori);
}


void load_board(int data[220]) {
  for (int i = 0; i < t_state.board.height; i++) {
    for (int j = 0; j < t_state.board.width; j++) {
      set_board_data(&t_state.committed_board, j, i, data[t_state.board.width * i + j]);
    }
  }
}

int te_is_game_over() {
  return t_state.game_over;
}
