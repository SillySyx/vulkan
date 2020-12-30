#ifdef __linux__
#include "window/xcb.h"
#else _WIN32
#include "window/win32.h"
#endif