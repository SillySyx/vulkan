#pragma once

// https://www.codeproject.com/articles/1089819/an-introduction-to-xcb-programming

#include <iostream>
#include <string.h>
#include <xcb/xcb.h>

#include "trace.h"
#include "window/options.h"

struct XcbWindow
{
    uint32_t window_id;
    xcb_connection_t *connection;
};

typedef XcbWindow WindowHandle;

WindowHandle xcb_window_create(WindowOptions *options);
void xcb_window_run_eventloop(WindowHandle *window, WindowOptions *options);
void xcb_window_set_title(WindowHandle *window, const char *title);
void xcb_window_set_mode(WindowHandle *window, WindowModes window_mode);
void xcb_window_set_size(WindowHandle *window, uint32_t width, uint32_t height);
void xcb_window_close(WindowHandle *window);

#define create_window xcb_window_create
#define run_window_eventloop xcb_window_run_eventloop
#define set_window_title xcb_window_set_title
#define set_window_mode xcb_window_set_mode
#define set_window_size xcb_window_set_size
#define close_window xcb_window_close