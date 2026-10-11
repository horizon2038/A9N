#!/bin/sh
set -eu
a9n_dir=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
test_dir=$(mktemp -d)
compiler=${CXX:-clang++}
sanitize_flags=
if [ "${SANITIZE:-0}" = 1 ]; then
    sanitize_flags='-fsanitize=address,undefined -fno-sanitize-recover=all'
fi
# local_apic_timer.cpp includes msr.hpp but uses no MSR operations. Skip its
# unused x86 inline assembly so the host regression also runs on ARM hosts.
"$compiler" -std=c++20 -O2 -DNDEBUG -DA9N_HAL_X86_64_ARCH_MSR_HPP \
    -fno-exceptions -fno-rtti $sanitize_flags \
    -I "$a9n_dir/src/hal/x86_64/include" \
    -I "$a9n_dir/src/hal/include" -I "$a9n_dir/src/kernel/include" \
    -I "$a9n_dir/src/liba9n/include" \
    "$a9n_dir/test/host/local-apic-timer.cpp" \
    "$a9n_dir/src/hal/x86_64/time/local_apic_timer.cpp" \
    -o "$test_dir/local-apic-timer"
"$test_dir/local-apic-timer"
printf 'Host test binary: %s\n' "$test_dir/local-apic-timer"
