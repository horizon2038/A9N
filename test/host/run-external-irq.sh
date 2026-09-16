#!/bin/sh
set -eu
a9n_dir=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
test_dir=$(mktemp -d)
compiler=${CXX:-clang++}
case $(uname -s) in
    Darwin) strip_flag=-Wl,-dead_strip ;;
    *) strip_flag=-Wl,--gc-sections ;;
esac
sanitize_flags=
if [ "${SANITIZE:-0}" = 1 ]; then
    sanitize_flags='-fsanitize=address,undefined -fno-sanitize-recover=all'
fi
for smp in 0 1; do
    smp_flag=
    if [ "$smp" = 1 ]; then smp_flag=-DA9N_CONFIG_ENABLE_SMP; fi
    "$compiler" -std=c++20 -O2 -DNDEBUG -fno-exceptions -fno-rtti \
        -ffunction-sections -fdata-sections -Dmemset=a9n_test_memset \
        $sanitize_flags $smp_flag \
        -I "$a9n_dir/src/hal/x86_64/include" \
        -I "$a9n_dir/src/hal/include" -I "$a9n_dir/src/kernel/include" \
        -I "$a9n_dir/src/liba9n/include" \
        "$a9n_dir/test/host/external-irq.cpp" \
        "$a9n_dir/src/kernel/interrupt/interrupt_manager.cpp" \
        "$strip_flag" -o "$test_dir/external-irq-$smp"
    "$test_dir/external-irq-$smp"
done
printf 'Host test binaries: %s\n' "$test_dir"
