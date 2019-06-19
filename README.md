# T-Engine
## A C/C++ Tetris engine

A C Tetris engine (2009 Tetris Guideline compliant-ish).
Bring your own input handling and rendering. See Examples.

## Features

- [x] I, O, T, S, Z, J, L pieces
- [x] Super Rotation System (SRS) (implementation variable but result still the same)
- [x] Random Generator ("random bag" or "7-system")
- [x] Ghost piece
- [x] "Hold piece"
- [x] Top out on overlapping spawn or partial lock out of bounds
- [x] Level up conditions
- [x] Scoring
- Gravity
  - [x] Naive (most games use this i.e. clear at most 4 lines at once)
  - [] Sticky
  - [] Cascade
- [x] Lock Delay

## TODO

- [ ] T-spin condition check
- [ ] Combos/Back-to-back
- [ ] Use a configuration struct to pass into `te_init_system()`
- [ ] Delay Auto Shift (DAS)
- [ ] Time-based update function (currently frame based)

## Data Structures

## API

```
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
```

## Example Implementation

## Resources

[Tetris Guidelines](https://tetris.wiki/Tetris_Guideline)

[Super Rotation System (SRS)](http://harddrop.com/wiki/SRS)

[T-Spin](http://harddrop.com/wiki/T-Spin)

[Line Clear](https://tetris.wiki/Line_clear)

[Combo](http://harddrop.com/wiki/Combo)

[Random Generator](http://harddrop.com/wiki/Random_Generator)

[Gravity Curve](https://harddrop.com/wiki/Tetris_Worlds)

[How Tetris 99 Gravity Works](https://old.reddit.com/r/Tetris99/comments/c03g2i/how_tetris_99_gravity_works)
