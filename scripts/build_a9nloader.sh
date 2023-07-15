#!/bin/bash

ARCH=${ARCH}
: ${ARCH:="x86_64"}
LLVMDIR=${LLVMDIR}
: ${LLVMDIR:="/usr/local/opt/llvm/bin"}
CURRENT=`dirname $0`

cd $CURRENT
rm -rf ../chain/$ARCH/edk2/a9nloaderPkg
cp -r ../src/boot/a9nloaderPkg ../chain/$ARCH/edk2/a9nloaderPkg
cd ../chain/$ARCH/edk2
EDKDIR=`pwd`
export PATH=$LLVMDIR:$PATH
export EDK_TOOLS_PATH=$EDKDIR/BaseTools
source ./edksetup.sh BaseTools
build
