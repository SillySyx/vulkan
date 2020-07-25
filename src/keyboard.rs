use winit::event::{ElementState, KeyboardInput, VirtualKeyCode};

pub type Keys = VirtualKeyCode;

#[derive(Debug, Clone)]
pub struct KeyboardState {
    pressed_keys: Vec<Keys>,
}

impl KeyboardState {
    pub fn new() -> Self {
        Self {
            pressed_keys: vec![],
        }
    }

    pub fn update(&mut self, input: &KeyboardInput) {
        if let KeyboardInput { virtual_keycode: Some(key), state, .. } = input {
            match state {
                ElementState::Pressed => self.press_key(key),
                ElementState::Released => self.release_key(key),
            };
        }
    }

    fn press_key(&mut self, key: &Keys) {
        self.pressed_keys.push(key.clone());
    }

    fn release_key(&mut self, key: &Keys) {
        let index = match self.pressed_keys.iter().position(|pressed_key| pressed_key == key) {
            Some(index) => index,
            None => return,
        };

        self.pressed_keys.remove(index);
    }

    pub fn is_key_pressed(&self, key: Keys) -> bool {
        match self.pressed_keys.iter().position(|pressed_key| *pressed_key == key) {
            Some(_) => true,
            None => false,
        }
    }
}