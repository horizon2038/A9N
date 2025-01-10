#!/bin/bash

ARCH=${ARCH}
: ${ARCH:="x86_64"}
LLVMDIR=${LLVMDIR}
: ${LLVMDIR:="/usr/local/opt/llvm/bin"}
CURRENT=`dirname $0`

cd $CURRENT
rm -rf ../chain/$ARCH/edk2/a9nloaderPkg
cp -r ../src/boot/a9nloaderPkg ../chain/$ARCH/edk2/a9nloaderPkg
cp -r ../src/boot/target.txt ../chain/$ARCH/edk2/Conf/
cd ../chain/$ARCH/edk2
EDKDIR=`pwd`
export PATH=$LLVMDIR:$PATH
export EDK_TOOLS_PATH=$EDKDIR/BaseTools
export CLANG_BIN="/usr/lib/llvm-16/bin/"
source ./edksetup.sh BaseTools
NUM_CPUS=8
build -n $NUM_CPUS
