use std::error::Error;

use vulkan::window::{Window, Keys};

fn main() -> Result<(), Box<dyn Error>> {
    let window = Window::new("window", 800, 600)?;

    window.run(|keyboard, quit| {
        if keyboard.is_key_pressed(Keys::Escape) {
            *quit = true;
        }
    });

    Ok(())
}