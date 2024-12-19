#!/bin/zsh

CURRENT=`dirname $0`
CURRENT=$(cd $(dirname $0);pwd)
cd $CURRENT

ARCH=${ARCH}
: ${ARCH:="x86_64"}
QEMU="qemu-system-"
QEMUFLAGS="-bios OVMF.fd -drive format=raw,file=fat:rw:./ -monitor stdio "
GDB_OPTION=
QEMUDIR=../run/$ARCH/QEMU
BUILDDIR=../build/$ARCH
# SERVERBUILDDIR=../src/servers/build/x86_64
SERVERBUILDDIR=$KOITO/target/x86_64-unknown-none/release

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
    cp $SERVERBUILDDIR/KOITO $QEMUDIR/kernel/init.elf
    cd $QEMUDIR
    # QEMUPARAM=("${QEMUFLAGS}" "${GDB_OPTION}")
    # qemu-system-$ARCH "${QEMUPARAM[@]}"
    qemu-system-${ARCH} \
        -drive if=pflash,format=raw,file=./OVMF_CODE.fd \
        -drive if=pflash,format=raw,file=./OVMF_VARS.fd \
        -drive format=raw,file=fat:rw:./ \
        -m 2G \
        -smp 8 \
        -M hpet=on \
        -nographic \
        -cpu Skylake-Client,+fsgsbase \
        -no-shutdown -no-reboot #-s -S
        # -s -S
        # -d int
        # -chardev stdio,id=char0,logfile=serial.log \
        # -serial chardev:char0
    # -monitor stdio
}

init
while getopts 'g' flag; do
    case "${flag}" in
        g) enable_gdb ;;
    esac
done
run_qemu
