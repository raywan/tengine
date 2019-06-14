#include <stdio.h>
#include "tengine.h"
#include <time.h>

#if defined(TENGINE_DEBUG)
#include <stdio.h>
#define ASSERT(expr) \
  if (expr) {} \
  else printf("ASSERT FAILED: %s(%d): %s\n", __FILE__, __LINE__, #expr)
#else
#define ASSERT(expr)
#endif

void render_board(Board *b) {
  puts("------------"); // Board width + 2 for border
  for (int i = 0; i < b->height; i++) {
    printf("|"); // Left side border
    for (int j = 0; j < b->width; j++) {
      if (b->data[b->width * i + j] == -1) {
        printf(" ");
      } else if (b->data[b->width * i + j] == 1) {
        printf("x");
      } else if (b->data[b->width * i + j] == 8) {
        printf("0");
      } else if (b->data[b->width * i + j] == -2) {
        printf("g");
      }
    }
    puts("|"); // Right side border
  }
  puts("------------"); // Board width + 2 for border
}

void clear_terminal() {
#if defined(_WIN32)
  system("cls");
#else
  printf("\e[1;1H\e[2J");
#endif
}

#if 0
int test_data[220] = {
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
8,8,-1,-1,-1,-1,-1,-1,-1,-1,
8,-1,-1,-1,-1,-1,-1,-1,-1,-1,
8,-1,8,8,8,-1,-1,-1,-1,-1,
8,-1,-1,8,8,8,8,8,-1,-1,
8,-1,8,8,8,8,8,8,8,8
};
#else
int test_data[220] = {
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,8,8,-1,-1,
-1,-1,-1,-1,-1,-1,-1,8,8,8,
8,8,8,8,8,-1,8,8,8,8,
};
#endif

int main(int argc, char *argv[]) {
  // ASSERT(1 == 0);
  puts("HELLO WORLD");
  unsigned int iteration = 0;
  te_init_system();
  Board *b = get_board();
  // load_board(test_data);
  Board *cb = get_committed_board();
  char c;
  int initial = 1;
  while (1) {
    c = getchar();
    if (c == 'a') {
      move_left();
    } else if (c == 'd') {
      move_right();
    } else if (c == 'z') {
      rotate_left();
    } else if (c == 'x') {
      rotate_right();
    } else if (c == 's') {
      move_down();
    } else if (c == ' ') {
      hard_drop();
    } else if (c == 'q') {
      hold();
    } else if (!initial) {
      continue;
    }
    initial = 0;
    // clear_terminal();
    puts("");
    te_update(0);
    render_board(b);
    puts("");
    render_board(cb);
    printf("iteration: %u\n", iteration++);
  }
  return 0;
}

