#ifndef DISPLAY_H_
#define DISPLAY_H_

/* The display size is not really configurable */
#define WIDTH  64
#define HEIGHT 32

extern uint64_t display[];

int initialize_display();
void clear_display();
void refresh_display();

#endif  // DISPLAY_H_
