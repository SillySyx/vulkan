#include "platform.h"
#include "renderer/vulkan.h"

int main()
{
    Device device;
    CreateDeviceInfo create_device_info;

    if (!create_device(&create_device_info, &device))
    {
        TRACE("FAILED to create device");
        return 0;
    }

    TRACE("device created");
}