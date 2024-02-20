#!/usr/bin/bash
# #!/bin/zsh

CURRENT=`dirname $0`
cd ${CURRENT}

BUILDDIR=../test/build

exit_code=0

for test in $@; do
    ${test}
    exit_code=$(( ${exit_code} | $? ))
done

exit $exit_code
