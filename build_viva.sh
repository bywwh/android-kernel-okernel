#!/bin/bash
##############################################################################
#
#                           Kernel Build Script
#
##############################################################################
# 2014-06-21 Shev_t       : modified
# 2011-10-24 effectivesky : modified
# 2010-12-29 allydrop     : created
##############################################################################

##############################################################################
# set toolchain
##############################################################################
export ARCH=arm
export SUBARCH=arm
export CROSS_COMPILE=~/AndroidSources/linaro-4.9.3/bin/arm-eabi-
export LOCALVERSION="-ok-015"
export CCACHE_DIR=~/.ccache/kernel
ccache -M 5G

##############################################################################
# set variables
##############################################################################
export KERNELDIR=`pwd`
KERNEL_OUT=$KERNELDIR/obj/KERNEL_OBJ
STRIP=${CROSS_COMPILE}strip

##############################################################################
# make zImage
##############################################################################
mkdir -p $KERNEL_OUT
mkdir -p $KERNEL_OUT/tmp/kernel

make O=$KERNEL_OUT viva_defconfig
make -j10 O=$KERNEL_OUT

if [ -f $KERNEL_OUT/arch/arm/boot/zImage ]
then
    cp -f $KERNEL_OUT/arch/arm/boot/zImage ./
    mv -f $KERNEL_OUT/arch/arm/boot/zImage $KERNEL_OUT/tmp/kernel/zImage
fi

if [ -f ./zImage ]
then
    CURRENT_DATE=`date +%Y%m%d-%H%M`
    cp ./android/blank_any_kernel.zip okernel$LOCALVERSION-$CURRENT_DATE.zip
    pushd $KERNEL_OUT/tmp
    zip -r ../../../okernel$LOCALVERSION-$CURRENT_DATE.zip ./kernel/
fi
