#include "window/xcb.h"

bool create_window(WindowOptions* options)
{
    auto connection = xcb_connect(NULL, NULL);
    if (xcb_connection_has_error(connection))
    {
        return false;
    }

    auto setup = xcb_get_setup(connection);
    auto screen = xcb_setup_roots_iterator(setup).data;

    options->window_id = xcb_generate_id(connection);
    auto value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;

    uint32_t value_list[] = 
    { 
        screen->black_pixel, 
        XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_RESIZE_REDIRECT | XCB_EVENT_MASK_FOCUS_CHANGE
    };

    xcb_create_window(connection, screen->root_depth, options->window_id, screen->root, options->top, options->left, options->width, options->height, 1, XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, value_mask, value_list);

    xcb_map_window(connection, options->window_id);
    xcb_flush(connection);

    xcb_generic_event_t *event;
    while (!options->shutdown && (event = xcb_wait_for_event(connection)))
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

    xcb_disconnect(connection);
    return true;
}

void set_window_title(uint32_t window_id, const char *title)
{
    auto connection = xcb_connect(NULL, NULL);
    if (xcb_connection_has_error(connection))
    {
        return;
    }

    xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window_id, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, strlen(title), title);

    xcb_disconnect(connection);
}

void set_window_mode_borderless(uint32_t window_id)
{
}

void set_window_mode_fullscreen(uint32_t window_id)
{
}

void set_window_mode_windowed(uint32_t window_id)
{
}

void set_window_size(uint32_t window_id, uint32_t width, uint32_t height)
{
}

void close_window(uint32_t window_id)
{
    auto connection = xcb_connect(NULL, NULL);
    if (xcb_connection_has_error(connection))
    {
        return;
    }

    xcb_destroy_window(connection, window_id);

    xcb_disconnect(connection);
}