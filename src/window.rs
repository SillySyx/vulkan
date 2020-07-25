use std::error::Error;
use std::{thread::sleep, time::Duration};

use winit::{
    window::WindowBuilder,
    dpi::LogicalSize,
    event_loop::{EventLoop, ControlFlow},
    platform::desktop::EventLoopExtDesktop,
    event::{Event, WindowEvent}, 
};

pub use crate::keyboard::{KeyboardState, Keys};

#[derive(Debug)]
pub struct Window {
    handle: winit::window::Window,
    event_loop: EventLoop<()>,
    keyboard_state: KeyboardState,
}

impl Window {
    pub fn new(title: &'static str, width: u32, height: u32) -> Result<Self, Box<dyn Error>> {
        let event_loop = EventLoop::new();

        let handle = WindowBuilder::new()
            .with_title(title)
            .with_inner_size(LogicalSize::new(width, height))
            .build(&event_loop)?;

        Ok(Window {
            handle,
            event_loop,
            keyboard_state: KeyboardState::new(),
        })
    }

    pub fn run<F: Fn(&KeyboardState, &mut bool) + 'static>(mut self, func: F) {
        let mut keyboard = self.keyboard_state;

        let mut quit = false;

        while !quit {
            self.event_loop
                .run_return(|event, _, control_flow| {
                    *control_flow = ControlFlow::Wait;

                    if should_stop_event_loop(&event) {
                        quit = true;
                    }

                    if let Event::MainEventsCleared { .. } = event {
                        *control_flow = ControlFlow::Exit;
                    }

                    if let Event::WindowEvent { event: WindowEvent::KeyboardInput { input, .. }, .. } = event {
                        keyboard.update(&input);
                    }
                });
    
            func(&keyboard, &mut quit);

            sleep(Duration::from_millis(16));
        }
    }
}

fn should_stop_event_loop<T>(event: &Event<T>) -> bool {
    match event {
        Event::WindowEvent { event: WindowEvent::CloseRequested, .. } => true,
        _ => false,
    }
}