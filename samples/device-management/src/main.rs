use vulkano::instance::{Instance, PhysicalDevice};
use vulkano::device::{Device, DeviceExtensions};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let required_extensions = vulkano_win::required_extensions();
    let instance = Instance::new(None, &required_extensions, None)?;

    for physical in PhysicalDevice::enumerate(&instance) {
        println!("Device: {} (type: {:?})", physical.name(), physical.ty());

        for queue_family in physical.queue_families() {
            println!("Queue: id={} supports_graphics={} supports_compute={} queues_count={}", queue_family.id(), queue_family.supports_graphics(), queue_family.supports_compute(), queue_family.queues_count());
        }

        let queue_family = physical
            .queue_families()
            .find(|&q| {
                q.supports_graphics()
            }).expect("oh no!");

        let device_ext = DeviceExtensions {
            khr_swapchain: true,
            ..DeviceExtensions::none()
        };

        let _ = Device::new(
            physical,
            physical.supported_features(),
            &device_ext,
            [(queue_family, 0.5)].iter().cloned(),
        )?;
    }

    Ok(())
}