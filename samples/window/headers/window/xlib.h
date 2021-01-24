#pragma once

#include <iostream>
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "trace.h"
#include "window/options.h"

struct XlibWindow
{
    Window window_id;
    Display *display;
};

typedef XlibWindow WindowHandle;

WindowHandle xlib_window_create(WindowOptions *options);
void xlib_window_run_eventloop(WindowHandle *window, WindowOptions *options);
void xlib_window_set_title(WindowHandle *window, const char *title);
void xlib_window_set_mode(WindowHandle *window, WindowModes window_mode);
void xlib_window_set_size(WindowHandle *window, uint32_t width, uint32_t height);
void xlib_window_close(WindowHandle *window);

#define create_window xlib_window_create
#define run_window_eventloop xlib_window_run_eventloop
#define set_window_title xlib_window_set_title
#define set_window_mode xlib_window_set_mode
#define set_window_size xlib_window_set_size
#define close_window xlib_window_close