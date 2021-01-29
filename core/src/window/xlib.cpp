#include "window/xlib.h"

WindowHandle xlib_window_create(WindowOptions *options)
{
    auto display = XOpenDisplay(NULL);
    auto screen = DefaultScreen(display);
    auto root = RootWindow(display, screen);
    auto depth = DefaultDepth(display, DefaultScreen(display));
    auto visual = DefaultVisual(display, DefaultScreen(display));
    auto border_width = 0;
    auto _class = InputOutput;
    auto attributes_mask = CWBackPixel;

    XSetWindowAttributes attributes;
    attributes.background_pixmap = ParentRelative;

    auto window = XCreateWindow(
        display,
        root,
        options->left,
        options->top,
        options->width,
        options->height,
        border_width,
        depth,
        _class,
        visual,
        attributes_mask,
        &attributes);

    XSelectInput(display, window, KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask | FocusChangeMask);
    XMapWindow(display, window);

    XFlush(display);

    return WindowHandle{
        window_id : window,
        display : display,
    };
}

void xlib_window_run_eventloop(WindowHandle *window, WindowOptions *options)
{
    while (!options->shutdown)
    {
        while (XPending(window->display) > 0)
        {
            XEvent xevent;
            XNextEvent(window->display, &xevent);

            if (xevent.type == DestroyNotify || xevent.type == UnmapNotify)
            {
                options->shutdown = true;
                break;
            }

            if (xevent.type == KeyPress)
            {
                auto key_code = xevent.xkey.keycode;

                if (options->key_pressed != NULL)
                    options->key_pressed(key_code);
            }

            if (xevent.type == ButtonPress)
            {
                auto button_code = xevent.xbutton.button;
                auto x = xevent.xbutton.x;
                auto y = xevent.xbutton.y;

                if (options->button_pressed != NULL)
                    options->button_pressed(button_code, x, y);
            }

            if (xevent.type == ConfigureNotify)
            {
                auto width = (uint32_t)xevent.xconfigure.width;
                auto height = (uint32_t)xevent.xconfigure.height;

                if (width == options->width && height == options->height)
                    continue;

                options->width = width;
                options->height = height;

                if (options->resized)
                    options->resized(width, height);
            }

            if (xevent.type == FocusIn) 
            {
                if (options->focused)
                    options->focused();
            }

            if (xevent.type == FocusOut) 
            {
                if (options->lost_focus)
                    options->lost_focus();
            }
        }
    }
}

void xlib_window_set_title(WindowHandle *window, const char *title)
{
    XChangeProperty(
        window->display, 
        window->window_id,
        XA_WM_NAME,
        XA_STRING, 
        8, 
        PropModeReplace, 
        (unsigned char *)title,
	    strlen(title)
    );
}

void xlib_window_set_fullscreen(WindowHandle *window)
{
    TRACE("xlib_window_set_fullscreen");

    XEvent xevent;
    xevent.xclient.type = ClientMessage;
    xevent.xclient.serial = 0;
    xevent.xclient.send_event = True;
    xevent.xclient.message_type = XInternAtom(window->display, "_NET_WM_STATE", False);
    xevent.xclient.window = window->window_id;
    xevent.xclient.format = 32;
    xevent.xclient.data.l[0] = 1;
    xevent.xclient.data.l[1] = XInternAtom(window->display, "_NET_WM_STATE_FULLSCREEN", False);

    XSendEvent(
        window->display, 
        DefaultRootWindow(window->display), 
        False, 
        SubstructureNotifyMask | SubstructureRedirectMask, 
        &xevent
    );
}

void xlib_window_set_windowed(WindowHandle *window)
{
    TRACE("xlib_window_set_windowed");

    XEvent xevent;
    xevent.xclient.type = ClientMessage;
    xevent.xclient.serial = 0;
    xevent.xclient.send_event = True;
    xevent.xclient.message_type = XInternAtom(window->display, "_NET_WM_STATE", False);
    xevent.xclient.window = window->window_id;
    xevent.xclient.format = 32;
    xevent.xclient.data.l[0] = 0;
    xevent.xclient.data.l[1] = XInternAtom(window->display, "_NET_WM_STATE_FULLSCREEN", False);

    XSendEvent(
        window->display, 
        DefaultRootWindow(window->display), 
        False, 
        SubstructureNotifyMask | SubstructureRedirectMask, 
        &xevent
    );
}

void xlib_window_set_mode(WindowHandle *window, WindowModes window_mode)
{
    if (window_mode == WindowModes::Fullscreen)
    {
        xlib_window_set_fullscreen(window);
    }

    if (window_mode == WindowModes::Windowed)
    {
        xlib_window_set_windowed(window);
    }
}

void xlib_window_set_size(WindowHandle *window, uint32_t width, uint32_t height)
{
    TRACE("xlib_window_set_size");

    auto flags = CWWidth | CWHeight;

    XWindowChanges changes;
    changes.width = width;
    changes.height = height;

    XConfigureWindow(window->display, window->window_id, flags, &changes);
}

void xlib_window_close(WindowHandle *window)
{
    XDestroyWindow(window->display, window->window_id);
}