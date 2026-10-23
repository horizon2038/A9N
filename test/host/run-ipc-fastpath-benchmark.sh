#!/bin/sh
set -eu
a9n_dir=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
test_dir=$(mktemp -d)
baseline_dir="$test_dir/baseline"
mkdir "$baseline_dir"
# Export into an isolated fixture; never reset or overwrite the worktree.
git -C "$a9n_dir" archive "${BASELINE_REF:-HEAD}" src | tar -xf - -C "$baseline_dir"
compiler=${CXX:-clang++}
case $(uname -s) in
    Darwin) strip_flag=-Wl,-dead_strip ;;
    *) strip_flag=-Wl,--gc-sections ;;
esac
for smp in 0 1; do
    smp_flag=
    if [ "$smp" = 1 ]; then smp_flag=-DA9N_CONFIG_ENABLE_SMP; fi
    for variant in baseline current; do
        source_dir="$a9n_dir"
        if [ "$variant" = baseline ]; then source_dir="$baseline_dir"; fi
        reclamation_source=
        if [ -f "$source_dir/src/kernel/capability/capability_node_reclamation.cpp" ]; then
            reclamation_source="$source_dir/src/kernel/capability/capability_node_reclamation.cpp"
        fi
        "$compiler" -std=c++20 -O2 -DNDEBUG $smp_flag \
            -fno-exceptions -fno-rtti -ffunction-sections -fdata-sections \
            -I "$source_dir/src/hal/x86_64/include" \
            -I "$source_dir/src/hal/include" -I "$source_dir/src/kernel/include" \
            -I "$source_dir/src/liba9n/include" \
            "$a9n_dir/test/host/ipc-fastpath-benchmark.cpp" \
            "$source_dir/src/kernel/capability/ipc_port.cpp" \
            "$source_dir/src/kernel/capability/notification_port.cpp" \
            "$source_dir/src/kernel/capability/capability_node.cpp" \
            $reclamation_source \
            "$strip_flag" -o "$test_dir/ipc-$variant-$smp"
    done
    # Reverse execution order in the second trial to expose simple order bias.
    for variant in baseline current current baseline; do
        "$test_dir/ipc-$variant-$smp" "$variant"
    done
done
printf 'Host benchmark fixtures: %s\n' "$test_dir"
