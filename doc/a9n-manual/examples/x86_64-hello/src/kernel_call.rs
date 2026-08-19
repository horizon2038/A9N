use crate::abi::{IpcBuffer, MESSAGE_BUFFER_SIZE_MAX};
use core::arch::asm;
use core::mem::align_of;
use core::ptr::{NonNull, read_volatile, write_volatile};

pub type MessageRegisters = [usize; 10];

pub struct VirtualMessageRegisters {
    register_backed: MessageRegisters,
    ipc_buffer: NonNull<IpcBuffer>,
}

impl VirtualMessageRegisters {
    /// `address` must refer to the mapped, exclusively accessible IPC Buffer
    /// assigned to the running process.
    pub unsafe fn from_ipc_buffer_address(address: usize) -> Option<Self> {
        if address == 0 || !address.is_multiple_of(align_of::<IpcBuffer>()) {
            return None;
        }

        Some(Self {
            register_backed: [0; 10],
            ipc_buffer: NonNull::new(address as *mut IpcBuffer)?,
        })
    }

    pub fn write(&mut self, index: usize, value: usize) -> bool {
        if let Some(register) = self.register_backed.get_mut(index) {
            *register = value;
            return true;
        }

        if index >= MESSAGE_BUFFER_SIZE_MAX {
            return false;
        }

        unsafe {
            write_volatile(self.ipc_buffer.cast::<usize>().as_ptr().add(index), value);
        }
        true
    }

    pub fn read(&self, index: usize) -> Option<usize> {
        if let Some(register) = self.register_backed.get(index) {
            return Some(*register);
        }

        if index >= MESSAGE_BUFFER_SIZE_MAX {
            return None;
        }

        unsafe {
            Some(read_volatile(
                self.ipc_buffer.cast::<usize>().as_ptr().add(index),
            ))
        }
    }

    pub fn clear_register_backed(&mut self) {
        self.register_backed = [0; 10];
    }

    pub unsafe fn enter_kernel(&mut self, number: isize) {
        unsafe {
            enter_kernel(number, &mut self.register_backed);
        }
    }
}

pub unsafe fn enter_kernel(number: isize, registers: &mut MessageRegisters) {
    let [mut mr0, mut mr1, mut mr2, mut mr3, mut mr4, mut mr5, mut mr6, mut mr7, mut mr8, mut mr9] =
        *registers;

    unsafe {
        asm!(
            "syscall",
            in("rax") number,
            inout("rdi") mr0,
            inout("rsi") mr1,
            inout("rdx") mr2,
            inout("r8") mr3,
            inout("r9") mr4,
            inout("r10") mr5,
            inout("r12") mr6,
            inout("r13") mr7,
            inout("r14") mr8,
            inout("r15") mr9,
            lateout("rcx") _,
            lateout("r11") _,
            options(nostack),
        );
    }

    *registers = [mr0, mr1, mr2, mr3, mr4, mr5, mr6, mr7, mr8, mr9];
}
