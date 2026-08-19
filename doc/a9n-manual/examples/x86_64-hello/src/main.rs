#![no_std]
#![no_main]

#[allow(dead_code)]
mod abi;
mod kernel_call;

use abi::InitInfo;
use core::arch::global_asm;
use core::panic::PanicInfo;
use kernel_call::{MessageRegisters, VirtualMessageRegisters, enter_kernel};

const DEBUG_CALL: isize = -3;
const YIELD_CALL: isize = -2;
const CAPABILITY_CALL: isize = -1;
const INIT_PCB_DESCRIPTOR: usize = 0x0801_0000_0000_0000;
const PCB_READ_REGISTER: usize = 2;
const PCB_REGISTER_READ_COUNT: usize = 8;
const IPC_BUFFER_SENTINEL: usize = 0xa9a9_a9a9_a9a9_a9a9;
const INVALID_DESCRIPTOR: usize = 2;

global_asm!(
    r#"
    .section .text.entry, "ax"
    .global _start
_start:
    lea rsp, [__init_stack_end]
    lea rdi, [__init_info_start]
    call rust_entry
1:
    mov rax, -2
    syscall
    jmp 1b
"#
);

#[unsafe(no_mangle)]
extern "C" fn rust_entry(init_info: *const InitInfo) -> ! {
    let init_info = unsafe { &*init_info };
    let Some(mut registers) = (unsafe {
        VirtualMessageRegisters::from_ipc_buffer_address(init_info.ipc_buffer)
    }) else {
        panic!();
    };

    write_bytes(b"A9N manual ABI check: OK\r\n");
    write_bytes(b"kernel version: 0x");
    write_hex(init_info.kernel_major_version);
    write_byte(b'.');
    write_hex(init_info.kernel_minor_version);
    write_byte(b'.');
    write_hex(init_info.kernel_patch_version);
    write_bytes(b"\r\n");

    if check_capability_call(&mut registers) {
        write_bytes(b"capability call check: OK\r\n");
    } else {
        write_bytes(b"capability call check: FAILED\r\n");
    }

    if check_capability_error(&mut registers) {
        write_bytes(b"capability error check: OK\r\n");
    } else {
        write_bytes(b"capability error check: FAILED\r\n");
    }

    yield_forever()
}

fn check_capability_call(registers: &mut VirtualMessageRegisters) -> bool {
    registers.clear_register_backed();
    let input_is_valid = registers.write(0, INIT_PCB_DESCRIPTOR)
        && registers.write(1, PCB_READ_REGISTER)
        && registers.write(2, PCB_REGISTER_READ_COUNT)
        && registers.write(10, IPC_BUFFER_SENTINEL);

    unsafe {
        registers.enter_kernel(CAPABILITY_CALL);
    }

    let result = input_is_valid
        && registers.read(0) == Some(1)
        && registers.read(10) != Some(IPC_BUFFER_SENTINEL);

    if !result {
        write_bytes(b"capability call MR0/MR1/MR10: 0x");
        write_hex(registers.read(0).unwrap_or(usize::MAX));
        write_byte(b'/');
        write_hex(registers.read(1).unwrap_or(usize::MAX));
        write_byte(b'/');
        write_hex(registers.read(10).unwrap_or(usize::MAX));
        write_bytes(b"\r\n");
    }

    result
}

fn check_capability_error(registers: &mut VirtualMessageRegisters) -> bool {
    registers.clear_register_backed();

    unsafe {
        registers.enter_kernel(CAPABILITY_CALL);
    }

    registers.read(0) == Some(0) && registers.read(1) == Some(INVALID_DESCRIPTOR)
}

fn write_bytes(bytes: &[u8]) {
    for &byte in bytes {
        write_byte(byte);
    }
}

fn write_hex(value: usize) {
    const DIGITS: &[u8; 16] = b"0123456789abcdef";
    let mut started = false;

    for shift in (0..usize::BITS).step_by(4).rev() {
        let digit = ((value >> shift) & 0xf) as usize;
        if digit != 0 || started || shift == 0 {
            started = true;
            write_byte(DIGITS[digit]);
        }
    }
}

fn write_byte(byte: u8) {
    let mut registers: MessageRegisters = [0; 10];
    registers[0] = byte as usize;

    unsafe {
        enter_kernel(DEBUG_CALL, &mut registers);
    }
}

fn yield_forever() -> ! {
    loop {
        let mut registers: MessageRegisters = [0; 10];
        unsafe {
            enter_kernel(YIELD_CALL, &mut registers);
        }
    }
}

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    write_bytes(b"A9N manual ABI check: PANIC\r\n");
    yield_forever()
}
