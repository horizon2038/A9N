pub const ARCH_INFO_MAX: usize = 128;
pub const INITIAL_GENERIC_COUNT_MAX: usize = 128;
pub const MESSAGE_BUFFER_SIZE_MAX: usize = 494;
pub const CAPABILITY_TRANSFER_COUNT_MAX: usize = 16;

#[repr(C)]
pub struct GenericDescriptor {
    pub address: usize,
    pub size_radix: u8,
    pub is_device: bool,
}

#[repr(C)]
pub struct InitInfo {
    pub kernel_major_version: usize,
    pub kernel_minor_version: usize,
    pub kernel_patch_version: usize,
    pub kernel_pre_release: [u8; 32],
    pub kernel_build_meta_data: [u8; 32],
    pub arch_info: [usize; ARCH_INFO_MAX],
    pub ipc_buffer: usize,
    pub generic_list: [GenericDescriptor; INITIAL_GENERIC_COUNT_MAX],
    pub generic_list_count: usize,
}

#[repr(C, align(4096))]
pub struct IpcBuffer {
    pub messages: [usize; MESSAGE_BUFFER_SIZE_MAX],
    pub transfer_source_descriptors: [usize; CAPABILITY_TRANSFER_COUNT_MAX],
    pub transfer_destination_node: usize,
    pub transfer_destination_index: usize,
}

const _: () = assert!(size_of::<InitInfo>() <= 4096);
const _: () = assert!(size_of::<IpcBuffer>() == 4096);
