#!/usr/bin/env bash

set -euo pipefail

readonly PROJECT_ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"

find "${PROJECT_ROOT}/src" \
    -type f \
    \( \
        -name '*.c' -o \
        -name '*.cc' -o \
        -name '*.cpp' -o \
        -name '*.cxx' -o \
        -name '*.h' -o \
        -name '*.hh' -o \
        -name '*.hpp' -o \
        -name '*.hxx' \
    \) \
    -exec clang-format -i {} +

# liba9n's libcxx headers intentionally have no filename extension.
find "${PROJECT_ROOT}/src/liba9n/include/liba9n/libcxx" \
    -type f ! -name '*.*' \
    -exec clang-format -i {} +
