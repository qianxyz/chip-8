#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <stdint.h>

int initialize_display();
void clear_display();
uint8_t draw_sprite(uint8_t col, uint8_t row, uint8_t *psprite, uint16_t n);
void terminate_display();

#endif  // DISPLAY_H_
