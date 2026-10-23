#!/bin/sh
set -eu
a9n_dir=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
test_dir=$(mktemp -d)
baseline_dir="$test_dir/baseline"
mkdir "$baseline_dir"
git -C "$a9n_dir" archive "${BASELINE_REF:-HEAD}" src | tar -xf - -C "$baseline_dir"
# Match the production toolchain instead of accidentally selecting Apple Clang.
llvm_bin=$(llvm-config --bindir)
compiler=${CODEGEN_CXX:-$llvm_bin/clang++}
objdump=${LLVM_OBJDUMP:-$llvm_bin/llvm-objdump}
for architecture in x86_64 aarch64; do
    architecture_flags='-mcpu=cortex-a72 -mgeneral-regs-only -mno-outline-atomics'
    if [ "$architecture" = x86_64 ]; then
        architecture_flags='-mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -mno-avx -mno-avx2'
    fi
  for optimization in O2 O3; do
    for variant in baseline current; do
        source_dir="$a9n_dir"
        if [ "$variant" = baseline ]; then source_dir="$baseline_dir"; fi
        "$compiler" --target="$architecture-unknown-none" -std=c++20 "-$optimization" -DNDEBUG \
            -ffreestanding -fno-exceptions -fno-rtti -fno-stack-protector \
            -fomit-frame-pointer -fno-pic -fno-pie $architecture_flags \
            -I "$source_dir/src/hal/$architecture/include" \
            -I "$source_dir/src/hal/include" -I "$source_dir/src/kernel/include" \
            -I "$source_dir/src/liba9n/include" \
            -c "$source_dir/src/kernel/capability/capability_node.cpp" \
            -o "$test_dir/$architecture-$optimization-$variant.o"
        "$objdump" -d --demangle --no-show-raw-insn "$test_dir/$architecture-$optimization-$variant.o" \
            | awk '/^[0-9a-f]+ <.*>:/ { keep = /capability_node::(traverse_slot|lookup_slot)\(/ } keep' \
            > "$test_dir/$architecture-$optimization-$variant.asm"
        awk '
            function finish() {
                if (name != "") { print name, instructions, branches }
            }
            /^[0-9a-f]+ <.*>:/ {
                finish()
                name = "lookup_slot"
                if (/::traverse_slot\(/) { name = "traverse_slot" }
                instructions = 0
                branches = 0
            }
            /^[[:space:]]*[0-9a-f]+:/ {
                opcode = $2
                if (opcode ~ /^nop/ || opcode == "int3") { next }
                instructions++
                if (opcode ~ /^j/ && opcode != "jmp" && opcode != "jmpq") { branches++ }
                if (opcode ~ /^b\./ || opcode ~ /^(cbz|cbnz|tbz|tbnz)$/) { branches++ }
            }
            END { finish() }
        ' "$test_dir/$architecture-$optimization-$variant.asm" \
            > "$test_dir/$architecture-$optimization-$variant.metrics"
    done
    awk -v architecture="$architecture" -v optimization="$optimization" '
        NR == FNR { instructions[$1] = $2; branches[$1] = $3; next }
        {
            if (!($1 in instructions) || $2 > instructions[$1] || $3 > branches[$1]) {
                print "FAIL: lookup code generation grew:", architecture, optimization, $0
                failed = 1
            }
            print architecture, optimization, $1, "instructions", instructions[$1] "->" $2,
                  "conditional-branches", branches[$1] "->" $3
            count++
        }
        END { if (failed || count != 2) { exit 1 } }
    ' "$test_dir/$architecture-$optimization-baseline.metrics" \
      "$test_dir/$architecture-$optimization-current.metrics"
  done
done
printf 'Production lookup disassembly: %s\n' "$test_dir"
