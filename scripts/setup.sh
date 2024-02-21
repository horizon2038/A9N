#!/bin/bash

# common setup
ARCH="x86_64"
CHAINDIR=../chain
SRCDIR=../src
RUNDIR=../run
TESTDIR=../test

CURRENT=`dirname $0`
CURRENT=$(cd $(dirname $0);pwd)
echo $CURRENT

# setup EDK2
pwd
cd $CURRENT
mkdir -p $CHAINDIR/$ARCH
cd $CHAINDIR/$ARCH
git clone https://github.com/tianocore/edk2.git
cd edk2
git submodule update --init
source edksetup.sh
make -C BaseTools/
cd $CURRENT
cp -f $SRCDIR/boot/target.txt $CHAINDIR/$ARCH/edk2/Conf/

# setup qemu
mkdir -p $RUNDIR/$ARCH/qemu
cp /usr/share/OVMF/OVMF_CODE.fd $RUNDIR/$ARCH/qemu/
cp /usr/share/OVMF/OVMF_VARS.fd $RUNDIR/$ARCH/qemu/

# setup google_test
mkdir -p $TESTDIR/library/include
cd $TESTDIR/library
git clone https://github.com/google/googletest.git
mkdir googletest/build
cd googletest/build
cmake ..
make
cd $CURRENT/$TESTDIR
cp -r library/googletest/googletest/include/gtest library/include
pwd
cp library/googletest/build/lib/*.a library/

