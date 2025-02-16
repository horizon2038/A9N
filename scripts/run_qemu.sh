#!/bin/bash

# inital configuration
CURRENT=`dirname $0`
CURRENT=$(cd $(dirname $0);pwd)
cd $CURRENT

ARCH=${ARCH}
: ${ARCH:="x86_64"}
QEMU="qemu-system-"

QEMUFLAGS=(
    # loading OVMFs
    "-drive"
    "if=pflash,format=raw,file=./OVMF_CODE.fd"
    "-drive"
    "if=pflash,format=raw,file=./OVMF_VARS.fd"

    # mount as virtual file system
    "-drive"
    "format=raw,file=fat:rw:./"

    "-nographic"

    # default memory size : 2 GiB
    "-m"
    "2G"
)

GDB_OPTION=
QEMUDIR=../run/$ARCH/QEMU
BUILDDIR=../build/$ARCH
SERVERBUILDDIR=$KOITO/target/x86_64-unknown-a9n/release

# functions
function init() {
    # CURRENT=`dirname $0`
    # cd $CURRENT
    echo QEMUDIR: $QEMUDIR
    echo BUILDDIR: $BUILDDIR
}

function help() {
    cat <<EOF
USAGE : run_qemu.sh [options]

OPTIONS:
    -g, --gdb               enable GDB debugging (localhost:1234)
    -c, --smp {count}       configure the number of CPUs
    -m, --memory {size}     specify the memory size (default : 2G)
    -i, --show-interrupt    displays interrupt information
    -n, --no-reload         turn off automatic restart in case of problems
    -h, --help              show this help message
EOF
}

function enable_gdb() {
    QEMUFLAGS+=("-s" "-S")
}

function set_smp() {
    QEMUFLAGS+=("-smp" "$1")
}

function set_memory_size() {
    local count="$1"
    local new_flags=()
    local skip=0

    for ((i=1; i <= ${#QEMUFLAGS[@]}; i++)); do
        local flag="${QEMUFLAGS[$i]}"
        if [[ "$skip" -gt 0 ]]; then
            skip=$((skip - 1))
            continue
        fi

        if [[ "$flag" == "-m" ]]; then
            skip=1
            continue
        fi

        new_flags+=("$flag")
    done

    QEMUFLAGS=("${new_flags[@]}")
    QEMUFLAGS+=("-m" "$count")
}

function enable_show_interrupt() {
    QEMUFLAGS+=("-d" "int")
}

function enable_no_reload() {
    QEMUFLAGS+=("-no-shutdown" "-no-reboot")
}

## architecture-dependent flags
# x86_64
function arch_add_x86_64_flags() {
    # configure CPU
    QEMUFLAGS+=("-cpu" "Skylake-Client,+fsgsbase")

    # enable HPET
    QEMUFLAGS+=("-M" "hpet=on")
}

function run_qemu() {
    mkdir -p $QEMUDIR/EFI/BOOT
    mkdir -p $QEMUDIR/kernel
    cp $BUILDDIR/boot/BOOTX64.EFI $QEMUDIR/EFI/BOOT
    cp $BUILDDIR/kernel/kernel.elf $QEMUDIR/kernel
    cp $SERVERBUILDDIR/KOITO $QEMUDIR/kernel/init.elf
    cd $QEMUDIR
    qemu-system-${ARCH} "${QEMUFLAGS[@]}"
}

# analyze options
function parse_options() {
    while [[ $# -gt 0 ]]; do
        case "$1" in
            -g|--gdb)
                enable_gdb
                shift
                ;;
            -c|--smp)
                if [[ -n "$2" && ! "$2" =~ ^- ]]; then
                    set_smp "$2"
                    shift 2
                else
                    echo "error: --smp requires a non-empty option argument."
                    exit 1
                fi
                ;;
            -m|--memory)
                if [[ -n "$2" && ! "$2" =~ ^- ]]; then
                    set_memory_size "$2"
                    shift 2
                else
                    echo "error: --memory requires a non-empty option argument."
                    exit 1
                fi
                ;;
            -i|--show-interrupt)
                enable_show_interrupt
                shift
                ;;
            -n|--no-reload)
                enable_no_reload
                shift
                ;;
            -h|--help)
                help
                exit 0
                ;;
            --) # End of all options
                shift
                break
                ;;
            -*) # Unknown option
                echo "Unknown option: $1"
                help
                exit 1
                ;;
            *) # No more options
                break
                ;;
        esac
    done
}

# initialize
init
arch_add_${ARCH}_flags

# parse arguments
parse_options "$@"

# run
run_qemu
