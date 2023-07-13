#ifndef MONITOR_H
#define MONITOR_H

#include "common/common.h"

#define COLOR_BLACK 0
#define COLOR_BLUE 1
#define COLOR_GREEN 2
#define COLOR_CYAN 3
#define COLOR_RED 4
#define COLOR_FUCHSINE 5
#define COLOR_BROWN 6
#define COLOR_WHITE 7

#define COLOR_LIGHT_BLACK 8
#define COLOR_LIGHT_BLUE 9
#define COLOR_LIGHT_GREEN 10
#define COLOR_LIGHT_CYAN 11
#define COLOR_LIGHT_RED 12
#define COLOR_LIGHT_FUCHSINE 13
#define COLOR_LIGHT_BROWN 14
#define COLOR_LIGHT_WHITE 15

// clear the screen
void monitor_init();

void monitor_clear();

// ********************** recommended print APIs **************************** //
void monitor_print(char* str);

void monitor_println(char* str);

void monitor_printf(char* str, ...);

void monitor_printf_args(char* str, void* args);

void monitor_print_with_color(char* str, uint8 color);

void monitor_move_cursor(int32 delta_x, int32 delta_y);

#endif
