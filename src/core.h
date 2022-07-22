#ifndef CORE_H_
#define CORE_H_

#include <stdint.h>

extern uint8_t  memory[];
extern uint16_t pc;
extern uint16_t I;
extern uint8_t  V[];

int run_emulator(char *);

#endif  // CORE_H_
