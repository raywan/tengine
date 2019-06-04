#include <stdio.h>
#include "tengine.h"

#define DEBUG

#if defined(DEBUG)
#include <stdio.h>
#define ASSERT(expr) \
  if (expr) {} \
  else printf("ASSERT FAILED: %s(%d): %s\n", __FILE__, __LINE__, #expr)
#else
#define ASSERT(expr)
#endif

void render_board(Board *b) {
  for (int i = 0; i < b->height; i++) {
    for (int j = 0; j < b->width; j++) {
      if (b->data[b->height * i + j] == -1) {
        printf("0");
      } else if (b->data[b->height * i + j] == 1) {
        printf("x");
      } else if (b->data[b->height * i + j] == 2) {
        printf("g");
      }
    }
    puts("");
  }
}



int main(int argc, char *argv[]) {
  // ASSERT(1 == 0);
  puts("HELLO WORLD");
  unsigned int iteration = 0;
  init_system();
  Board *b = get_board();
  char c;
  int initial = 1;
  while (1) {
    c = getchar();
    if (c == 'a') {
      move_left();
    } else if (c == 'd') {
      move_right();
    } else if (c == 'w') {
      rotate_right();
    } else if (c == 's') {
      move_down();
    } else if (!initial) {
      continue;
    }
    initial = 0;
    printf("\e[1;1H\e[2J");
    puts("");
    update();
    render_board(b);
    printf("iteration: %u\n", iteration++);

  }
  return 0;
}

