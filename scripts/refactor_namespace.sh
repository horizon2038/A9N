#!/bin/bash

# 置き換えを行うディレクトリを指定
TARGET_DIR=$1

# 確認メッセージ
echo "Processing files in directory: $TARGET_DIR"

find "$TARGET_DIR" -type f \( -name "*.cpp" -o -name "*.hpp" \) |\
    xargs \
    gsed -i \
        -e 's/#include <liba9n\/common\/types.hpp>/#include <kernel\/types.hpp>/g'
echo "Replacement complete."

# find "$TARGET_DIR" -type f \( -name "*.cpp" -o -name "*.hpp" \)
