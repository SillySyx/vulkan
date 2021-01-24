#pragma once

#include <functional>

struct WindowOptions {
    bool shutdown = false;
    uint32_t width = 1920;
    uint32_t height = 1080;
    uint32_t top = 0;
    uint32_t left = 0;

    std::function<void(uint32_t)> key_pressed;
    std::function<void(uint32_t, uint32_t, uint32_t)> button_pressed;
    std::function<void(uint32_t, uint32_t)> resized;
    std::function<void(void)> focused;
    std::function<void(void)> lost_focus;
};

enum WindowModes {
    Fullscreen,
    Borderless,
    Windowed,
};