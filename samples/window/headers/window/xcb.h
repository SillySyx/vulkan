#pragma once

// https://www.codeproject.com/articles/1089819/an-introduction-to-xcb-programming

#include <iostream>
#include <string.h>
#include <xcb/xcb.h>

#include "window/options.h"

class XcbWindow
{
public:
    uint32_t window_id;
    xcb_connection_t *connection;
};

XcbWindow create_window(WindowOptions* options);

void run_window_eventloop(XcbWindow* window, WindowOptions* options);

void set_window_title(XcbWindow* window, const char *title);

void set_window_mode_borderless(XcbWindow* window);

void set_window_mode_fullscreen(XcbWindow* window);

void set_window_mode_windowed(XcbWindow* window);

void set_window_size(XcbWindow* window, uint32_t width, uint32_t height);

void close_window(XcbWindow* window);