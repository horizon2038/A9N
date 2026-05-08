#!/bin/bash

# 置き換えを行うディレクトリを指定
TARGET_DIR=$1

# 確認メッセージ
echo "Processing files in directory: $TARGET_DIR"

find "$TARGET_DIR" -type f \( -name "*.cpp" -o -name "*.hpp" \) |\
    xargs \
    gsed -i \
        -e 's/capability_error_type/capet/g'

echo "Replacement complete."
