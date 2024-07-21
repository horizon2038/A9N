#!/bin/bash

A9NDIR=../

CURRENT=`dirname $0`
CURRENT=$(cd $(dirname $0);pwd)
cd $CURRENT
cd $A9NDIR

docker run --rm -v $(pwd):/A9N a9n-build bash -c "make -j8"
