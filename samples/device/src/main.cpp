#include "platform.h"
#include "renderer/vulkan.h"

int main()
{
    CreateDeviceInfo create_device_info;
    create_device_info.application_name = "test";
    create_device_info.engine_name = "test";
    create_device_info.enableValidation = false;

    Device device;
    if (!create_device(&create_device_info, &device))
    {
        TRACE("FAILED to create device");
        return 0;
    }

    destroy_device(&device);
}