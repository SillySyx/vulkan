#if defined(__linux__)
#define USE_XLIB_WINDOW 1
#define VK_USE_PLATFORM_XLIB_KHR 1
// #define USE_XCB_WINDOW 1
// #define VK_USE_PLATFORM_XCB_KHR 1
#endif

#if defined(_WIN32)
#define USE_WIN32_WINDOW 1
#define VK_USE_PLATFORM_WIN32_KHR 1
#endif