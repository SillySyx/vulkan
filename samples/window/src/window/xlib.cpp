#include "window/xlib.h"

WindowHandle xlib_window_create(WindowOptions *options)
{
    TRACE("XOpenDisplay");
    auto display = XOpenDisplay(NULL);

    TRACE("DefaultRootWindow");
    auto root_screen = DefaultRootWindow(display);

    XSetWindowAttributes attrs;
    attrs.override_redirect = true;

    XVisualInfo vinfo;
    if (!XMatchVisualInfo(display, DefaultScreen(display), 32, TrueColor, &vinfo))
    {
        TRACE("No visual found supporting 32 bit color, terminating");
        exit(EXIT_FAILURE);
    }

    TRACE("XCreateWindow");
    auto window = XCreateWindow(
        display,
        root_screen,
        0,
        0,
        200,
        200,
        0,
        vinfo.depth,
        InputOutput,
        vinfo.visual,
        CWOverrideRedirect | CWColormap | CWBackPixel | CWBorderPixel,
        &attrs);

    TRACE("XSelectInput");
    XSelectInput(display, window, KeyPressMask | KeyReleaseMask);

    TRACE("XMapWindow");
    XMapWindow(display, window);

    TRACE("XFlush");
    XFlush(display);

    return WindowHandle{
        window_id : window,
        display : display,
    };
}

void xlib_window_run_eventloop(WindowHandle *window, WindowOptions *options)
{
    TRACE("xlib_window_run_eventloop");

    while (!options->shutdown && (XPending(window->display)))
    {
        TRACE("XPending");

        XEvent xevent;
        TRACE("XNextEvent");
        XNextEvent(window->display, &xevent);

        if (xevent.type == DestroyNotify)
        {
            break;
        }

        if (xevent.type == UnmapNotify)
        {
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

        // Window xWindow = xEvent.xany.window;

        // Event event;
        // event.base.timeMS = GetRunTimeMS();
        // event.base.canvas = 0;
        // if (m_canvasMap.count(xWindow))
        //     event.base.canvas = m_canvasMap[xWindow];

        // switch (xEvent.type)
        // {
        // case ClientMessage:
        //     if (xEvent.xclient.data.l[0] == m_wmDeleteMessage)
        //         return Stop();
        //     break;
        // case DestroyNotify:
        // case UnmapNotify:
        //     return Stop(true);
        // case KeyPress:
        //     event.type = event::KeyDown;
        //     event.key.scanCode = NativeScanCodeToScanCode(xEvent.xkey.keycode);
        //     event.key.keyCode = ScanCodeToKeyCode(event.key.scanCode);
        //     event.key.state = xEvent.xkey.state;
        //     OnEvent(event);
        //     break;
        // case KeyRelease:
        //     event.type = event::KeyUp;
        //     event.key.scanCode = NativeScanCodeToScanCode(xEvent.xkey.keycode);
        //     event.key.keyCode = ScanCodeToKeyCode(event.key.scanCode);
        //     event.key.state = xEvent.xkey.state;
        //     OnEvent(event);
        //     break;
        //     ;
        // case ButtonPress:
        //     event.mouseButton.type = event::MouseButtonDown;
        //     switch (xEvent.xbutton.button)
        //     {
        //     case 1:
        //         event.mouseButton.button = event::MouseButtonLeft;
        //         break;
        //     case 2:
        //         event.mouseButton.button = event::MouseButtonMiddle;
        //         break;
        //     case 3:
        //         event.mouseButton.button = event::MouseButtonRight;
        //         break;
        //     default:
        //         event.mouseButton.button = event::Unknown;
        //     }
        //     event.mouseButton.state = xEvent.xbutton.state;
        //     event.mouseButton.x = xEvent.xbutton.x;
        //     event.mouseButton.y = xEvent.xbutton.y;
        //     OnEvent(event);
        //     break;
        // case ButtonRelease:
        //     event.mouseButton.type = event::MouseButtonUp;
        //     switch (xEvent.xbutton.button)
        //     {
        //     case 1:
        //         event.mouseButton.button = event::MouseButtonLeft;
        //         break;
        //     case 2:
        //         event.mouseButton.button = event::MouseButtonMiddle;
        //         break;
        //     case 3:
        //         event.mouseButton.button = event::MouseButtonRight;
        //         break;
        //     default:
        //         event.mouseButton.button = event::Unknown;
        //     }
        //     event.mouseButton.state = xEvent.xbutton.state;
        //     event.mouseButton.x = xEvent.xbutton.x;
        //     event.mouseButton.y = xEvent.xbutton.y;
        //     OnEvent(event);
        //     break;
        // case MotionNotify:
        //     event.type = event::MouseMotion;
        //     event.mouseMotion.x = xEvent.xmotion.x;
        //     event.mouseMotion.y = xEvent.xmotion.y;
        //     OnEvent(event);
        //     break;
        // case ConfigureNotify:
        //     if (xEvent.xconfigure.width != event.base.canvas->GetWidth() || xEvent.xconfigure.height != event.base.canvas->GetHeight())
        //     {
        //         event.base.canvas->UpdateViewport(xEvent.xconfigure.width, xEvent.xconfigure.height);
        //         event.type = event::WindowResize;
        //         event.windowResize.width = xEvent.xconfigure.width;
        //         event.windowResize.height = xEvent.xconfigure.height;
        //         OnEvent(event);
        //     }
        //     break;
        // }
    }
}

void xlib_window_set_title(WindowHandle *window, const char *title)
{
    TRACE("xlib_window_set_title");
}

void xlib_window_set_mode(WindowHandle *window, WindowModes window_mode)
{
    TRACE("xlib_window_set_mode");
}

void xlib_window_set_size(WindowHandle *window, uint32_t width, uint32_t height)
{
    TRACE("xlib_window_set_size");
}

void xlib_window_close(WindowHandle *window)
{
    TRACE("xlib_window_close");
}