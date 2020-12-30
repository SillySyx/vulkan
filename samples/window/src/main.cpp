#include "window.h"

int main()
{
    WindowOptions options = {
        key_pressed: [&](uint32_t key_code) {
            std::cout << "clicked key " << key_code << std::endl;

            if (key_code == 10) {
                set_window_title(options.window_id, "first");
            }

            if (key_code == 11) {
                set_window_title(options.window_id, "second");
            }

            if (key_code == 9) {
                options.shutdown = true;
            }
        },
        button_pressed: [](uint32_t button_code, uint32_t x, uint32_t y) {
            std::cout << "clicked button " << button_code << " at " << x << " x " << y << std::endl;
        },
        resized: [](uint32_t width, uint32_t height) {
            std::cout << "resized " << width << " x " << height << std::endl;
        },
        focused: []() {
            std::cout << "focused" << std::endl;
        },
        lost_focus: []() {
            std::cout << "lost focus" << std::endl;
        },
    };

    if (!create_window(&options))
    {
        std::cout << "Failed to create window!" << std::endl;
    }

    set_window_title(options.window_id, "window test");

    // start event loop thingy
    // wait for event loop thread to terminate
}