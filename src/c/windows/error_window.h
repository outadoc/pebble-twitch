#pragma once

#include <pebble.h>

void error_window_push(const char *title, const char *message);
void error_window_pop(void);
