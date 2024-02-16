![A9N_header](https://github.com/horizon2038/A9N/assets/89717194/1ad9d635-c165-4609-bf47-cd592998409f)

# A9N microkernel

![LLVM](https://img.shields.io/badge/ASM-00599C?style=for-the-badge&logoColor=white)
![C](https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white)
![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![LLVM](https://img.shields.io/badge/llvm-00599C?style=for-the-badge&logo=llvm&logoColor=white)
![EDK2](https://img.shields.io/badge/%2Aedk2-00599C?style=for-the-badge&logoColor=white)

A9N is a **capability-based microkernel** that supports a variety of hardware platforms through appropriate **HAL**.  
It is implemented with an object-oriented interface, making it easy to use and extend.  
It combines high portability, stability, and scalability.

## A9N Components list

<pre>
.
└── src
    ├── boot
    ├── hal
    │    └── include/hal/interface
    │    └── {ARCH}
    ├── kernel
    ├── library
    └── servers
</pre>

### `src/boot`

A basic bootloader implementation is provided to load the kernel.  
This bootloader is currently only implemented for the x86_64 by EDK2.

### `src/hal`

A Hardware Abstraction Layer (HAL) is implemented to provide a portable interface
to the underlying hardware.  
The {ARCH} directory is referenced during the `make` process.  
> [!NOTE]
> The bootloader binary is **separated** from the kernel binary.

### `src/kernel`

The main hardware-independent part of the A9N microkernel.

### `src/library`

The A9N base library.
Used by the kernel, HAL, and user.

### `src/servers`

A test root server that is started by the A9N microkernel.  

> [!IMPORTANT]
> **Please keep in mind that the initial implementation is a simple one for testing purposes only,  
> and you will need to implement most of the functionality yourself.  
> This implementation will vary depending on the OS that uses the A9N kernel as its core.**

## Architecture Status

Currently supported architectures:

- x86_64 (Long Mode)

## Requirements

#### Kernel ( Hardware Independent )

- clang
- clang++
- lld

#### x86_64

- NASM
- [EDK2](https://github.com/tianocore/edk2)

## Build

```bash
make
```

## Usage

```bash
cd scripts/
./run.sh
```

## Author

horizon2k38 ( Rekka "horizon" Igumi )

email : [address](<mailto:rekka728@gmail.com>)  
X : [@horizon2k38](https://x.com/horizon2k38)  
Mastodon : [@horizon2k38@mstdn.jp](https://mstdn.jp/@horizon2k38)  
Misskey : [@horizon](https://misskey.io/@horizon)  

## Acknowledgements

[Mitou Jr](https://jr.mitou.org) : This project was supported by the Mitou Junior program.  
@kyasbal : My mentor during the Mitou Junior program, who provided valuable advice.  
@nuta : Gave me a advice on the implementation.  

And I would also like to thank everyone who supported this project.  

## License

[MIT License](https://choosealicense.com/licenses/mit/)
