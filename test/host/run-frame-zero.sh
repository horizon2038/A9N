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
for architecture in x86_64 aarch64; do
    architecture_flag=
    if [ "$architecture" = aarch64 ]; then architecture_flag=-DTEST_AARCH64; fi
    "$compiler" -std=c++20 -O2 -DNDEBUG -fno-exceptions -fno-rtti \
        -ffunction-sections -fdata-sections $sanitize_flags $architecture_flag \
        -I "$a9n_dir/test/host/include" \
        -I "$a9n_dir/src/hal/$architecture/include" \
        -I "$a9n_dir/src/hal/include" -I "$a9n_dir/src/kernel/include" \
        -I "$a9n_dir/src/liba9n/include" \
        "$a9n_dir/test/host/frame-zero.cpp" \
        "$a9n_dir/src/hal/$architecture/memory/memory_manager.cpp" \
        "$a9n_dir/src/kernel/capability/frame_capability.cpp" \
        "$a9n_dir/src/kernel/capability/generic.cpp" \
        "$strip_flag" -o "$test_dir/frame-zero-$architecture"
    "$test_dir/frame-zero-$architecture"
done
printf 'Host test binaries: %s\n' "$test_dir"
