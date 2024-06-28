#!/bin/bash

# 置き換えを行うディレクトリを指定
TARGET_DIR=$1

# 確認メッセージ
echo "Processing files in directory: $TARGET_DIR"

# 置き換えコマンド
# find "$TARGET_DIR" -type f \( -name "*.cpp" -o -name "*.hpp" \) |\
#     xargs \
#     gsed -i \
#         -e 's/\bkernel::\b/a9n::kernel::/g' \
#         -e 's/\bhal::\b/a9n::hal::/g' \
#         -e 's/\blibrary::\b/liba9n::/g' \
#         -e 's/\bcommon::word\b/a9n::word/g' \
#         -e 's/\bcommon::sword\b/a9n::sword/g' \
#         -e 's/\bcommon::virtual_address\b/a9n::virtual_address/g' \
#         -e 's/\bcommon::physical_address\b/a9n::physical_address/g'

find "$TARGET_DIR" -type f \( -name "*.cpp" -o -name "*.hpp" \) |\
    xargs \
    gsed -i \
        -e 's/\bliba9n::common::PAGE_SIZE\b/a9n::PAGE_SIZE/g' \
        -e 's/\bcommon::PAGE_SIZE\b/a9n::PAGE_SIZE/g'
echo "Replacement complete."

# find "$TARGET_DIR" -type f \( -name "*.cpp" -o -name "*.hpp" \)
