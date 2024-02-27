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
├── src
│   ├── kernel
│   ├── hal
│   │    └── include/hal/interface
│   │    └── {ARCH}
│   ├── boot
│   ├── library
│   └── servers
└── test

</pre>

### `src/kernel`

The main hardware-independent part of the A9N microkernel.

### `src/hal`

A Hardware Abstraction Layer (HAL) is implemented to provide a portable interface
to the underlying hardware.  
The {ARCH} directory is referenced during the `make` process.  

### `src/boot`

A basic bootloader implementation is provided to load the kernel.  
This bootloader is currently only implemented for the x86_64 by EDK2.
> [!NOTE]
> The bootloader binary is **separated** from the kernel binary.

### `src/library`

The A9N base library.
Used by the kernel, HAL, and user.

### `src/servers`

A test root server that is started by the A9N microkernel.  

> [!IMPORTANT]
> **Please keep in mind that the initial implementation is a simple one for testing purposes only,  
> and you will need to implement most of the functionality yourself.  
> This implementation will vary depending on the OS that uses the A9N kernel as its core.**

### `test`

This directory contains the kernel test code, which uses the *Google Test* framework.  
The tests are automatically built and run when you run `make`.  
You can also build them explicitly by running `make test`.

## Architecture Status

Currently supported architectures:

- x86_64 (Long Mode)

## Requirements

### Kernel

- Clang
- Clang++
- lld

### HAL

**x86_64**
- NASM

### Boot

**x86_64**
- [EDK2](https://github.com/tianocore/edk2)

### Test

- [Google Test](https://github.com/google/googletest)

### Run
- QEMU

## Build (with Docker)

```bash
docker build -t a9n-build .
docker run --rm -v $(pwd):/A9N a9n-build bash -c "./scripts/setup.sh && make -j8"
```

## Build (Local)

Building with Docker is recommended; This method is **deprecated**.

``` bash
sudo apt update && sudo apt install -y \
    bash \
    llvm \
    clang \
    lld \
    nasm \
    make \
    cmake \
    build-essential \
    uuid-dev \
    iasl \
    git \
    python-is-python3 \
    ovmf
./scripts/setup.sh
make -j8
```

## Usage

```bash
./scripts/run.sh
```

## Author

horizon2k38 ( Rekka "horizon" Igumi )

email : [address](<mailto:rekka728@gmail.com>)  
X : [@horizon2k38](https://x.com/horizon2k38)  
Mastodon : [@horizon2k38@mstdn.jp](https://mstdn.jp/@horizon2k38)  
Misskey : [@horizon](https://misskey.io/@horizon)  

## Acknowledgements

[Mitou Jr](https://jr.mitou.org) : This project was supported by the Mitou Junior program.  
[@kyasbal](https://github.com/kyasbal) : My mentor during the Mitou Junior program, who provided valuable advice.  
[@nuta](https://github.com/nuta) : Gave me a advice on the implementation.  

And I would also like to thank everyone who supported this project.  

## License

[MIT License](https://choosealicense.com/licenses/mit/)
