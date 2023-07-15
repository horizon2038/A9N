#!/bin/zsh

#Build and copy .EFI file

function init () {
    #Init ENV
    CURRENT=$(pwd)
    EDK2=~/Documents/Program/A9N/A9N/chain/x86_64/edk2
    echo $SHELL

    #init LLVM
    export PATH=/usr/local/opt/llvm/bin:$PATH

    #Init color
    ESC=$(printf '\033')
    BLUE="[34;47m"

    #init option
    GDB_OPTION=""

    #Log_1
    echo "EDK2 Folder: ${EDK2}"
}

function enable_build () {
    cd ${EDK2}
    source ./edksetup.sh BaseTools
    build
    # cd ${CURRENT}
    # cp ${EDK2}/build/a9nloaderPkg/x64/DEBUG_CLANGPDB/X64/a9nloader.efi EFI/BOOT/BOOTX64.EFI
    printf "${ESC}${BLUE}OK: BUILD SUCCESSFUL\n${ESC}[m"
}

function enable_gdb () {
    GDB_OPTION="-s -S"
}

function run_qemu () {
    #Run QEMU
    QEMU_RUN_SCRIPT="qemu-system-x86_64 -bios OVMF.fd -drive format=raw,file=fat:rw:./ -monitor stdio $GDB_OPTION"
    echo ${QEMU_RUN_SCRIPT}
    eval ${QEMU_RUN_SCRIPT}
}

#parse option
init
while getopts 'bg' flag; do
    case "${flag}" in
        b) enable_build ;;
        g) enable_gdb ;;
    esac
done
run_qemu