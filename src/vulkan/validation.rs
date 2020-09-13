use std::error::Error;
use std::os::raw::c_char;
use std::ffi::CStr;

use ash::{
    Entry,
    version::EntryV1_0,
};

pub fn is_validation_layers_supported(entry: &Entry, validation_layers: &Vec<&str>) -> Result<bool, Box<dyn Error>> {
    let layer_properties = entry.enumerate_instance_layer_properties()?;

    if layer_properties.len() <= 0 {
        return Ok(false);
    }

    for layer_name in validation_layers.iter() {
        let mut is_layer_found = false;

        for layer_property in layer_properties.iter() {
            let test_layer_name = vk_to_string(&layer_property.layer_name)?;
            if (*layer_name) == test_layer_name {
                is_layer_found = true;
                break;
            }
        }

        if is_layer_found == false {
            return Ok(false);
        }
    }

    return Ok(true);
}

fn vk_to_string(raw_string_array: &[c_char]) -> Result<String, Box<dyn Error>> {
    let raw_string = unsafe {
        let pointer = raw_string_array.as_ptr();
        CStr::from_ptr(pointer)
    };

    Ok(raw_string.to_str()?.to_owned())
}