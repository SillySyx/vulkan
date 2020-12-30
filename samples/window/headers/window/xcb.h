#pragma once

// https://www.codeproject.com/articles/1089819/an-introduction-to-xcb-programming

#include <iostream>
#include <string.h>
#include <xcb/xcb.h>

#include "window/options.h"

bool create_window(WindowOptions* options);

void set_window_title(uint32_t window_id, const char *title);

void set_window_mode_borderless(uint32_t window_id);

void set_window_mode_fullscreen(uint32_t window_id);

void set_window_mode_windowed(uint32_t window_id);

void set_window_size(uint32_t window_id, uint32_t width, uint32_t height);

void close_window(uint32_t window_id);