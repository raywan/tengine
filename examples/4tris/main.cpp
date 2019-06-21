#include <stdio.h>
#include <string>
#include <assert.h>
#include <vector>
#include <unordered_map>
#include <stdint.h>
#include <windows.h>
#include <SDL.h>
#include <SDL_main.h>
#include <SDL_ttf.h>
#include "../../tengine.h"

constexpr SDL_Color WHITE = { 255, 255, 255 };
constexpr int WINDOW_WIDTH = 540;
constexpr int WINDOW_HEIGHT = 480;
constexpr int BOARD_WIDTH = 10;
constexpr int BOARD_HEIGHT = 20;
constexpr int OUTER_PADDING = 20; // px
// For shifting the board over 
constexpr int BOARD_TOP_LEFT_X = 150; // px
constexpr int BOARD_TOP_Y = 20; // px

constexpr int FRAMES_PER_SECOND = 60;

constexpr float square_wh = (float) (WINDOW_HEIGHT - 2*OUTER_PADDING) / BOARD_HEIGHT;

TTF_Font *font;

void set_square_color(SDL_Renderer *renderer, PieceType type, bool is_ghost) {
  int opacity = is_ghost ? 64 : 255;
  switch (type) {
    case PT_I: 
      SDL_SetRenderDrawColor(renderer, 0, 255, 255, opacity);
      break;
    case PT_O:
      SDL_SetRenderDrawColor(renderer, 255, 255, 0, opacity);
      break;
    case PT_T:
      SDL_SetRenderDrawColor(renderer, 128, 0, 128, opacity);
      break;
    case PT_S:
      SDL_SetRenderDrawColor(renderer, 0, 255, 0, opacity);
      break;
    case PT_Z:
      SDL_SetRenderDrawColor(renderer, 255, 0, 0, opacity);
      break;
    case PT_J:
      SDL_SetRenderDrawColor(renderer, 0, 0, 255, opacity);
      break;
    case PT_L:
      SDL_SetRenderDrawColor(renderer, 255, 125, 0, opacity);
      break;
    default:
      break;
  }
}

void render_text(SDL_Renderer *renderer, const char* text, int x, int y) {
  SDL_Surface *surface = TTF_RenderText_Blended(font, text, WHITE);
  SDL_Rect dest = { x, y, surface->w, surface->h };
  SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_RenderCopy( renderer, tex, NULL, &dest );
  SDL_DestroyTexture(tex);
  SDL_FreeSurface(surface);
}

// Renders a piece onto the screen
// Used for rendering pieces off the board
void render_piece(SDL_Renderer* renderer, PieceType type, PieceOrientation ori, int x, int y, float scale) {
  PieceOffsets offsets = te_get_piece_offsets(type, ori);
  float wh = scale * square_wh;
  set_square_color(renderer, type, false);
	SDL_Rect r1 = { x, y, wh, wh };
	SDL_Rect r2 = { x + (wh * offsets.ox_1), y + (wh * offsets.oy_1), wh, wh };
	SDL_Rect r3 = { x + (wh * offsets.ox_2), y + (wh * offsets.oy_2), wh, wh };
	SDL_Rect r4 = { x + (wh * offsets.ox_3), y + (wh * offsets.oy_3), wh, wh };
  SDL_Rect piece_rects[4] = { r1, r2, r3, r4 };
	SDL_RenderFillRects(renderer, piece_rects, 4);
  SDL_SetRenderDrawColor(renderer, 33, 33, 33, 255);
  // SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderDrawRects(renderer, piece_rects, 4);
}

void render_board(SDL_Renderer *renderer, TState *t_state) {
  Board *board = te_get_board();
  std::unordered_map<PieceType, std::vector<SDL_Rect>> rect_map;
  for (int i = 0; i < board->height; i++) {
    for (int j = 0; j < board->width; j++) {
			if (te_get_board_xy(board, j, i) != -1) {
				SDL_Rect rect = { BOARD_TOP_LEFT_X + square_wh * j, BOARD_TOP_Y + square_wh * i, square_wh, square_wh };
        PieceType key = (PieceType) board->data[board->width * i + j];
        if (rect_map.find(key) != rect_map.end()) {
          rect_map[key].push_back(rect);
        } else {
          std::vector<SDL_Rect> v({ rect });
          rect_map[key] = v;
        }
		 	}
    }
  }
  // Loop through all the piece types
  for (auto it = rect_map.begin(); it != rect_map.end(); it++) {
    if (it->first == PT_GHOST) {
      set_square_color(renderer, t_state->cur_piece.type, true);
    } else {
      set_square_color(renderer, it->first, false);
    }
    SDL_RenderFillRects(renderer, it->second.data(), it->second.size());
  }
}

void render_ui(SDL_Renderer* renderer, TState *t_state) {
	// Draw horizontal grid lines
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
	for (int i = 0; i < BOARD_HEIGHT+1; i++) {
		// x1, y1, x2, y2
		SDL_RenderDrawLine(renderer, 
			BOARD_TOP_LEFT_X, 
			OUTER_PADDING + (square_wh*i), 
			BOARD_TOP_LEFT_X + (square_wh * BOARD_WIDTH), 
			OUTER_PADDING + (square_wh*i)
		);
	}

	// Draw vertical grid lines
	for (int i = 0; i < BOARD_WIDTH+1; i++) {
		// x1, y1, x2, y2
		SDL_RenderDrawLine(renderer, 
			BOARD_TOP_LEFT_X + (square_wh*i),
			OUTER_PADDING, 
			BOARD_TOP_LEFT_X + (square_wh*i),
			WINDOW_HEIGHT-OUTER_PADDING
		);
	}

	// Draw the hold area
	SDL_Rect hold_rect = { 45, 45, 75, 75 };
	SDL_RenderDrawRect(renderer, &hold_rect);
  if (!t_state->is_initial_swap) {
    render_piece(renderer, t_state->held_piece.type, PO_ZERO, 45 + 25, 45 + 25, 0.6);
  }

	// Draw the next piece area
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
  int next_rect_x = BOARD_TOP_LEFT_X + OUTER_PADDING + square_wh * BOARD_WIDTH;
  int next_rect_y = OUTER_PADDING + 30;
	SDL_Rect next_rect = { next_rect_x, next_rect_y, 75, 270 };
	SDL_RenderDrawRect(renderer, &next_rect);
  int buf_len = 0;
  PieceType* next_piece_buf = te_get_next_piece_buf(&buf_len);
  for (int i = 0; i < buf_len; i++) {
    render_piece(renderer, next_piece_buf[i], PO_ZERO,
      next_rect_x + 25,
      next_rect_y + 50 * (i+1),
      0.6
    );
  }

  // Render text
  render_text(renderer, "HOLD", 45, 30);
  render_text(renderer, "NEXT", BOARD_TOP_LEFT_X + OUTER_PADDING + square_wh * BOARD_WIDTH, 30);
  // Score/Level
  render_text(renderer, "LEVEL", 45, 200);
  render_text(renderer, std::to_string(t_state->level).c_str(), 45, 215);
  render_text(renderer, "LINES", 45, 250);
  render_text(renderer, std::to_string(t_state->num_lines_cleared).c_str(), 45, 265);
  render_text(renderer, "SCORE", 45, 300);
  render_text(renderer, std::to_string(t_state->score).c_str(), 45, 315);
}

void render(SDL_Renderer *renderer, TState *t_state) {
	SDL_SetRenderDrawColor(renderer, 33, 33, 33, 255);
	SDL_RenderClear(renderer);

	render_board(renderer, t_state);
	render_ui(renderer, t_state);

	SDL_RenderPresent(renderer);
}

void update(int frames) {
  if (te_is_game_over()) {
    // TODO(ray):
    puts("GAME OVER");
  }
  else {
    te_update(frames);
  }
}

int main(int argc, char *argv[]) {
	puts("Hello");
	te_init_system();
	TState *t_state = te_get_state();
	printf("w: %d, h: %d\n", t_state->board.width, t_state->board.height);

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		return 1;
	}

  if (TTF_Init() == -1) {
    return 2;
  }

  font = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", 13);

	SDL_Window* window = SDL_CreateWindow("4tris", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	if (window == NULL) {
		SDL_Quit();
		return 1;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL) {
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	uint64_t frames = 0;
	SDL_Event e;
	bool quit = false;
	while (!quit) {
		int start_ticks = SDL_GetTicks();
		while (SDL_PollEvent(&e)) {
			switch (e.type) {
				case SDL_QUIT:
					quit = true;
					break;
				case SDL_KEYDOWN:
          switch (e.key.keysym.sym) {
            case SDLK_ESCAPE:
              quit = true;
            case SDLK_LEFT:
              te_move_left();
              break;
            case SDLK_RIGHT:
              te_move_right();
              break;
            case SDLK_DOWN:
              te_move_down();
              break;
            case SDLK_SPACE:
              te_hard_drop();
              break;
            case SDLK_z:
              te_rotate_left();
              break;
            case SDLK_x:
              te_rotate_right();
              break;
            case SDLK_r:
              te_init_system();
              break;
            case SDLK_LSHIFT:
              te_hold();
              break;
          }
					break;
				case SDL_KEYUP:
					break;
				case SDL_MOUSEMOTION:
					break;
				case SDL_MOUSEWHEEL:
					break;
				default:
					break;
			}
		}

		update(frames);

		render(renderer, t_state);

		++frames;

		int frame_ticks = SDL_GetTicks() - start_ticks;
		if (frame_ticks < 1000 / FRAMES_PER_SECOND) {
      // printf("%d %d %d\n", frame_ticks, 1000 / FRAMES_PER_SECOND, 1000 / FRAMES_PER_SECOND - frame_ticks);
			SDL_Delay((1000/FRAMES_PER_SECOND) - frame_ticks);
		}
		++frames;
	}

  te_destroy_system();
  TTF_CloseFont(font);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}