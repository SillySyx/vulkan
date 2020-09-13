use std::{
    error::Error,
    os::raw::c_void,
    ffi::CString,
    ptr,
};

use ash::{
    version::EntryV1_0,
    Instance,
    Entry, 
    extensions::ext::DebugUtils,
    extensions::khr::Surface,
    vk,
    vk::ApplicationInfo,
    vk::StructureType,
    vk::make_version,
};

use super::{
    debug::{is_debug_mode, create_debug_messenger_info},
    validation::is_validation_layers_supported,
};

pub fn create_vk_instance(entry: &Entry, application_name: &'static str, engine_name: &'static str, validation_layers: Vec<&str>) -> Result<Instance, Box<dyn Error>> {
    if !is_validation_layers_supported(entry, &validation_layers)? {
        return Err(Box::from("Requested validation layers are not supported"));
    }

    let app_info = create_application_info(application_name, engine_name)?;

    let p_next = match is_debug_mode() {
        true => &create_debug_messenger_info() as *const vk::DebugUtilsMessengerCreateInfoEXT as *const c_void,
        false => ptr::null(),
    };

    let validation_layers_names: Vec<*const i8> = validation_layers
        .iter()
        .map(|layer| CString::new(*layer).unwrap().as_ptr())
        .collect();

    let extension_names = vec![
        Surface::name().as_ptr(),
        // platform specific surface?!?
        DebugUtils::name().as_ptr(),
    ];

    let create_info = vk::InstanceCreateInfo {
        s_type: StructureType::INSTANCE_CREATE_INFO,
        p_next: p_next,
        flags: vk::InstanceCreateFlags::empty(),
        p_application_info: &app_info,
        pp_enabled_layer_names: validation_layers_names.as_ptr(),
        enabled_layer_count: validation_layers_names.len() as u32,
        pp_enabled_extension_names: extension_names.as_ptr(),
        enabled_extension_count: extension_names.len() as u32,
    };

    let instance: Instance = unsafe {
        entry.create_instance(&create_info, None)?
    };

    Ok(instance)
}

fn create_application_info(application_name: &'static str, engine_name: &'static str) -> Result<ApplicationInfo, Box<dyn Error>> {
    let s_type = StructureType::APPLICATION_INFO;
    let p_next = ptr::null();
    let api_version = make_version(1, 0, 92);

    let p_application_name = CString::new(application_name)?.as_ptr();
    let application_version = make_version(1, 0, 0);

    let p_engine_name = CString::new(engine_name)?.as_ptr();
    let engine_version = make_version(1, 0, 0);

    Ok(ApplicationInfo {
        s_type,
        p_next,
        p_application_name,
        application_version,
        p_engine_name,
        engine_version,
        api_version,
    })
}

