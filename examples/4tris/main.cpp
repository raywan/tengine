#include <stdio.h>
#include <vector>
#include <stdint.h>
#include <windows.h>
#include <SDL.h>
#include <SDL_main.h>
#include "../../tengine.h"

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define BOARD_WIDTH 10
#define BOARD_HEIGHT 22
#define OUTER_PADDING 20 // px
// For shifting the board over 
#define BOARD_TOP_LEFT_X 150 // px
#define BOARD_TOP_Y 20 // px

#define FRAMES_PER_SECOND 60

constexpr float square_wh = (float) (WINDOW_HEIGHT - 2*OUTER_PADDING) / BOARD_HEIGHT;

void render_cur_piece(SDL_Renderer *renderer) {
	SDL_SetRenderDrawColor(renderer, 0, 255, 255, SDL_ALPHA_OPAQUE);
	SDL_Rect next_rect = { BOARD_TOP_LEFT_X + square_wh * 0, square_wh * 5, square_wh, square_wh };
	SDL_Rect piece_rects[4] = { next_rect, next_rect, next_rect, next_rect };
	SDL_RenderFillRects(renderer, piece_rects, 4);
}

void render_board(SDL_Renderer *renderer, Board* board) {
	std::vector<SDL_Rect> rects;
  for (int i = 0; i < board->height; i++) {
    for (int j = 0; j < board->width; j++) {
			if (board->data[board->width * i + j] != -1) {
				SDL_Rect next_rect = { BOARD_TOP_LEFT_X + square_wh * j, BOARD_TOP_Y + square_wh * i, square_wh, square_wh };
				switch (board->data[board->width * i + j]) {
					case -2: {
						SDL_SetRenderDrawColor(renderer, 255, 0, 255, 128);
					} break;
					case PT_I: {
						// SDL_SetRenderDrawColor(renderer, 0, 255, 255, SDL_ALPHA_OPAQUE);
					} break;
					default:
						break;
				}
				rects.push_back(next_rect);
		 	}
    }
  }
	SDL_RenderFillRects(renderer, rects.data(), rects.size());
	SDL_RenderPresent(renderer);
}

void render_ui(SDL_Renderer* renderer) {
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
	SDL_RenderPresent(renderer);

	// Draw the next piece area
	SDL_Rect next_rect = { BOARD_TOP_LEFT_X + OUTER_PADDING + square_wh * BOARD_WIDTH, OUTER_PADDING, 75, 250 };
	SDL_RenderDrawRect(renderer, &next_rect);
	SDL_RenderPresent(renderer);
}

void render(SDL_Renderer *renderer, TState *t_state) {

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

	// render_cur_piece(renderer);
	render_board(renderer, &t_state->board);
	render_ui(renderer);
}

void update(int frames) {
	ts_update(frames);
}

int main(int argc, char *argv[]) {
	puts("Hello");
	init_system();
	TState *t_state = get_state();
	printf("w: %d, h: %d\n", t_state->board.width, t_state->board.height);

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		return 1;
	}

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
			SDL_Delay((1000/FRAMES_PER_SECOND) - frame_ticks);
		}
		++frames;
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}