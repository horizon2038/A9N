# A9NLoader

A9NLoader is the bootloader designed to launch the A9N Kernel.

## Usage

The A9NLoader obtains the memory map and places the `kernel.elf` appropriately in memory.
While it's primarily intended for booting the A9N Kernel, it's capable of launching other kernels in ELF format as well.
In such cases, ensure your kernel is designed to receive the address of the `boot_info` structure.

## Build

Ensure EDK2 is properly set up. Specify `a9nloaderPkg/a9nloader.dsc` in `edk2/Conf/target.txt` and then execute the build command.

```bash
build
```

## License

[MIT License](https://choosealicense.com/licenses/mit/)
