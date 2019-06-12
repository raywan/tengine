#include <stdio.h>
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

#define FRAMES_PER_SECOND 60

constexpr float square_wh = (float) (WINDOW_HEIGHT - 2*OUTER_PADDING) / BOARD_HEIGHT;

void render_ui(SDL_Renderer* renderer) {
	// Draw horizontal grid lines
	for (int i = 0; i < BOARD_HEIGHT+1; i++) {
		// x1, y1, x2, y2
		SDL_RenderDrawLine(renderer, 
			BOARD_TOP_LEFT_X + OUTER_PADDING, 
			OUTER_PADDING + (square_wh*i), 
			BOARD_TOP_LEFT_X + OUTER_PADDING + (square_wh * BOARD_WIDTH), 
			OUTER_PADDING + (square_wh*i)
		);
	}

	// Draw vertical grid lines
	for (int i = 0; i < BOARD_WIDTH+1; i++) {
		// x1, y1, x2, y2
		SDL_RenderDrawLine(renderer, 
			BOARD_TOP_LEFT_X + OUTER_PADDING + (square_wh*i),
			OUTER_PADDING, 
			BOARD_TOP_LEFT_X + OUTER_PADDING + (square_wh*i),
			WINDOW_HEIGHT-OUTER_PADDING
		);
	}

	// Draw the hold area
	SDL_Rect hold_rect = { 45, 45, 75, 75 };
	SDL_RenderDrawRect(renderer, &hold_rect);
	SDL_RenderPresent(renderer);
}

void render(SDL_Renderer *renderer) {

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

	render_ui(renderer);
}

void update() {
	// TODO(ray):
}

int main(int argc, char *argv[]) {
	puts("Hello");
	init_system();

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

	uint64_t frames = 0;
	SDL_Event e;
	bool quit = false;
	while (!quit) {
		int start_ticks = SDL_GetTicks();
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT || e.type == SDL_KEYDOWN) quit = true;
		}

		update();

		render(renderer);

		++frames;

		int frame_ticks = SDL_GetTicks() - start_ticks;
		if (frame_ticks < 1000 / FRAMES_PER_SECOND) {
			SDL_Delay((1000/FRAMES_PER_SECOND) - frame_ticks);
		}
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}