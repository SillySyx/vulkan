#include <string.h>
#include <iostream>

#include "platform.h"
#include "window.h"

int main()
{
    WindowHandle window;

    WindowOptions options = {
        key_pressed: [&](uint32_t key_code) {
            std::cout << "clicked key " << key_code << std::endl;

            if (key_code == 10) {
                set_window_title(&window, "first");
            }

            if (key_code == 11) {
                set_window_title(&window, "second");
            }

            if (key_code == 12) {
                set_window_mode(&window, WindowModes::Borderless);
            }

            if (key_code == 13) {
                set_window_mode(&window, WindowModes::Fullscreen);
            }

            if (key_code == 14) {
                set_window_mode(&window, WindowModes::Windowed);
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

    window = create_window(&options);

    set_window_title(&window, "window test");

    run_window_eventloop(&window, &options);

    close_window(&window);
}