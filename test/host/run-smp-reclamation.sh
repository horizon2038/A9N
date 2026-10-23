#!/bin/sh
set -eu
a9n_dir=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
test_dir=$(mktemp -d)
compiler=${CXX:-clang++}
"$compiler" -std=c++20 -O1 -g -DNDEBUG -DA9N_CONFIG_ENABLE_SMP \
    -fsanitize=address,undefined -fno-sanitize-recover=all -pthread \
    -fno-exceptions -fno-rtti \
    -I "$a9n_dir/src/hal/x86_64/include" -I "$a9n_dir/src/hal/include" \
    -I "$a9n_dir/src/kernel/include" -I "$a9n_dir/src/liba9n/include" \
    "$a9n_dir/test/host/smp-reclamation.cpp" \
    "$a9n_dir/src/kernel/process/lock.cpp" \
    "$a9n_dir/src/kernel/process/reclamation.cpp" -o "$test_dir/smp-reclamation"
"$test_dir/smp-reclamation"
printf 'Host test binary: %s\n' "$test_dir/smp-reclamation"
