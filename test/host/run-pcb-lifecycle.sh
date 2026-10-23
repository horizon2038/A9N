#!/bin/sh
set -eu
a9n_dir=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
source_dir=${A9N_TEST_SOURCE_DIR:-$a9n_dir}
test_dir=$(mktemp -d)
compiler=${CXX:-clang++}
case $(uname -s) in
    Darwin) strip_flag=-Wl,-dead_strip ;;
    *) strip_flag=-Wl,--gc-sections ;;
esac
for smp in 0 1; do
    smp_flag=
    if [ "$smp" = 1 ]; then smp_flag=-DA9N_CONFIG_ENABLE_SMP; fi
    "$compiler" -std=c++20 -O1 -g -DNDEBUG $smp_flag -fno-exceptions -fno-rtti \
        -fsanitize=address,undefined -fno-sanitize-recover=all \
        -ffunction-sections -fdata-sections \
        -I "$source_dir/src/hal/x86_64/include" -I "$source_dir/src/hal/include" \
        -I "$source_dir/src/kernel/include" -I "$source_dir/src/liba9n/include" \
        "$a9n_dir/test/host/pcb-lifecycle.cpp" \
        "$source_dir/src/kernel/capability/process_control_block.cpp" \
        "$source_dir/src/kernel/capability/capability_node.cpp" \
        "$source_dir/src/kernel/capability/capability_node_reclamation.cpp" \
        "$strip_flag" -o "$test_dir/pcb-lifecycle-$smp"
    "$test_dir/pcb-lifecycle-$smp"
done
printf 'Host test binaries: %s\n' "$test_dir"
