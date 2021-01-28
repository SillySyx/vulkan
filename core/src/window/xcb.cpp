#include "window/xcb.h"

WindowHandle xcb_window_create(WindowOptions *options)
{
    TRACE("xcb_window_create");

    XcbWindow window;

    window.connection = xcb_connect(NULL, NULL);
    if (xcb_connection_has_error(window.connection))
    {
        throw "asd";
    }

    auto setup = xcb_get_setup(window.connection);
    auto screen = xcb_setup_roots_iterator(setup).data;

    window.window_id = xcb_generate_id(window.connection);
    auto value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;

    uint32_t value_list[] =
        {
            screen->black_pixel,
            XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_RESIZE_REDIRECT | XCB_EVENT_MASK_FOCUS_CHANGE};

    xcb_create_window(window.connection, screen->root_depth, window.window_id, screen->root, options->top, options->left, options->width, options->height, 1, XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, value_mask, value_list);

    xcb_map_window(window.connection, window.window_id);
    xcb_flush(window.connection);

    return window;
}

void xcb_window_run_eventloop(WindowHandle *window, WindowOptions *options)
{
    TRACE("xcb_window_run_eventloop");

    while (!options->shutdown)
    {
        xcb_generic_event_t *event;
        while ((event = xcb_wait_for_event(window->connection)))
        {
            if (event->response_type == XCB_KEY_PRESS)
            {
                auto key_code = ((xcb_key_press_event_t *)event)->detail;

                if (options->key_pressed != NULL)
                    options->key_pressed(key_code);
            }

            if (event->response_type == XCB_BUTTON_PRESS)
            {
                auto button_code = ((xcb_button_press_event_t *)event)->detail;
                auto x = ((xcb_button_press_event_t *)event)->event_x;
                auto y = ((xcb_button_press_event_t *)event)->event_y;

                if (options->button_pressed != NULL)
                    options->button_pressed(button_code, x, y);
            }

            if (event->response_type == XCB_RESIZE_REQUEST)
            {
                auto width = ((xcb_resize_request_event_t *)event)->width;
                auto height = ((xcb_resize_request_event_t *)event)->height;

                if (options->resized)
                    options->resized(width, height);
            }

            if (event->response_type == XCB_FOCUS_IN)
            {
                if (options->focused)
                    options->focused();
            }

            if (event->response_type == XCB_FOCUS_OUT)
            {
                if (options->lost_focus)
                    options->lost_focus();
            }

            free(event);
        }
    }
}

void xcb_window_set_title(WindowHandle *window, const char *title)
{
    TRACE("xcb_window_set_title");

    xcb_change_property(window->connection, XCB_PROP_MODE_REPLACE, window->window_id, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, strlen(title), title);
    xcb_flush(window->connection);
}

static inline xcb_intern_atom_reply_t *intern_helper(xcb_connection_t *conn, bool only_if_exists, const char *str)
{
    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(conn, only_if_exists, strlen(str), str);
    return xcb_intern_atom_reply(conn, cookie, NULL);
}

void xcb_window_set_fullscreen(WindowHandle *window)
{
    TRACE("xcb_window_set_fullscreen");

    xcb_intern_atom_reply_t *atom_wm_state = intern_helper(window->connection, false, "_NET_WM_STATE");
    xcb_intern_atom_reply_t *atom_wm_fullscreen = intern_helper(window->connection, false, "_NET_WM_STATE_FULLSCREEN");

    xcb_change_property(window->connection, XCB_PROP_MODE_REPLACE, window->window_id, atom_wm_state->atom, XCB_ATOM, 32, 1, &(atom_wm_fullscreen->atom));

    xcb_map_window(window->connection, window->window_id);
    xcb_flush(window->connection);

    free(atom_wm_fullscreen);
    free(atom_wm_state);
}

void xcb_window_set_borderless(WindowHandle *window)
{
    TRACE("xcb_window_set_borderless");
}

void xcb_window_set_windowed(WindowHandle *window)
{
    TRACE("xcb_window_set_windowed");
}

void xcb_window_set_mode(WindowHandle *window, WindowModes window_mode)
{
    if (window_mode == WindowModes::Fullscreen)
    {
        xcb_window_set_fullscreen(window);
    }

    if (window_mode == WindowModes::Borderless)
    {
        xcb_window_set_borderless(window);
    }

    if (window_mode == WindowModes::Windowed)
    {
        xcb_window_set_windowed(window);
    }
}

void xcb_window_set_size(WindowHandle *window, uint32_t width, uint32_t height)
{
    TRACE("xcb_window_set_size");
}

void xcb_window_close(WindowHandle *window)
{
    TRACE("xcb_window_close");

    xcb_destroy_window(window->connection, window->window_id);

    xcb_disconnect(window->connection);
}