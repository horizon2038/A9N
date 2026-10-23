#!/bin/sh
set -eu
a9n_dir=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
test_dir=$(mktemp -d)
compiler=${CXX:-clang++}
case $(uname -s) in
    Darwin) strip_flag=-Wl,-dead_strip ;;
    *) strip_flag=-Wl,--gc-sections ;;
esac
"$compiler" -std=c++20 -O1 -g -DNDEBUG -fsanitize=address,undefined \
    -fno-sanitize-recover=all -fno-exceptions -fno-rtti \
    -ffunction-sections -fdata-sections \
    -I "$a9n_dir/src/hal/x86_64/include" \
    -I "$a9n_dir/src/hal/include" -I "$a9n_dir/src/kernel/include" \
    -I "$a9n_dir/src/liba9n/include" \
    "$a9n_dir/test/host/capability-reclamation.cpp" \
    "$a9n_dir/src/kernel/capability/capability_node.cpp" \
    "$a9n_dir/src/kernel/capability/capability_node_reclamation.cpp" \
    "$strip_flag" -o "$test_dir/capability-reclamation"
"$test_dir/capability-reclamation"
printf 'Host test binary: %s\n' "$test_dir/capability-reclamation"
