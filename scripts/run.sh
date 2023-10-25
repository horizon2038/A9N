#!/bin/zsh

ARCH=${ARCH}
: ${ARCH:="x86_64"}
QEMU="qemu-system-"
QEMUFLAGS="-bios OVMF.fd -drive format=raw,file=fat:rw:./ -monitor stdio"
GDB_OPTION=
QEMUDIR=../run/$ARCH/QEMU
BUILDDIR=../build/$ARCH

function init() {
    CURRENT=`dirname $0`
    cd $CURRENT
    echo QEMUDIR: $QEMUDIR
    echo BUILDDIR: $BUILDDIR
}

function enable_gdb() {
    GDB_OPTION="-s -S"
}

function run_qemu() {
    mkdir -p $QEMUDIR/EFI/BOOT
    mkdir -p $QEMUDIR/kernel
    cp $BUILDDIR/boot/BOOTX64.EFI $QEMUDIR/EFI/BOOT
    cp $BUILDDIR/kernel/kernel.elf $QEMUDIR/kernel
    cd $QEMUDIR
    # QEMUPARAM=("${QEMUFLAGS}" "${GDB_OPTION}")
    # qemu-system-$ARCH "${QEMUPARAM[@]}"
    qemu-system-${ARCH} -bios OVMF.fd -drive format=raw,file=fat:rw:./ -serial mon:stdio -m 2G -s -S
    # -monitor stdio
}

init
while getopts 'g' flag; do
    case "${flag}" in
        g) enable_gdb ;;
    esac
done
run_qemu
