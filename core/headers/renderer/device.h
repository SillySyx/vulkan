#pragma once

#include <vector>
#include <vulkan/vulkan.h>

#include "trace.h"

struct Device {
    VkInstance instance = VK_NULL_HANDLE;
};

struct CreateDeviceInfo {
    const char * application_name;
    const char * engine_name;
    bool enableValidation;
};

bool create_device(CreateDeviceInfo* create_device_info, Device* device);