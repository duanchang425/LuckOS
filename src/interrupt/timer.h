#ifndef INTERRUPT_IRQ_H
#define INTERRUPT_IRQ_H

#include "common/common.h"

#define TIMER_FREQUENCY 50

void init_timer(uint32 frequency);

uint32 getTick();

#endif
