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
export CROSS_COMPILE=~/AndroidSources/linaro-4.9-2014.10-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
export LOCALVERSION="-ok-011"
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
mkdir -p $KERNEL_OUT/tmp/system/lib/modules

make O=$KERNEL_OUT front_defconfig
make -j10 O=$KERNEL_OUT

if [ -f $KERNEL_OUT/arch/arm/boot/zImage ]
then
    cp -f $KERNEL_OUT/arch/arm/boot/zImage ./
    mv -f $KERNEL_OUT/arch/arm/boot/zImage $KERNEL_OUT/tmp/kernel/zImage
fi

##############################################################################
# make SGX module
##############################################################################
if [ -f ./zImage ]
then
    make clean -C $KERNELDIR/sgx/pvr-source/eurasiacon/build/linux2/omap4430_android
    make -j10 -C $KERNELDIR/sgx/pvr-source/eurasiacon/build/linux2/omap4430_android KERNELDIR=$KERNEL_OUT TARGET_PRODUCT="blaze" BUILD=release TARGET_SGX=540 PLATFORM_VERSION=4.0
    mv $KERNELDIR/sgx/pvr-source/eurasiacon/binary2_540_120_omap4430_android_release/target/kbuild/pvrsrvkm_sgx540_120.ko $KERNEL_OUT
    $STRIP --strip-unneeded $KERNEL_OUT/pvrsrvkm_sgx540_120.ko
    make clean -C $KERNELDIR/sgx/pvr-source/eurasiacon/build/linux2/omap4430_android
    rm -r $KERNELDIR/sgx/pvr-source/eurasiacon/binary2_540_120_omap4430_android_release
    cp $KERNEL_OUT/pvrsrvkm_sgx540_120.ko ./pvrsrvkm_sgx540_120.ko
    mv $KERNEL_OUT/pvrsrvkm_sgx540_120.ko $KERNEL_OUT/tmp/system/lib/modules/pvrsrvkm_sgx540_120.ko
    CURRENT_DATE=`date +%Y%m%d-%H%M`
    cp ./android/blank_any_kernel.zip okernel$LOCALVERSION-$CURRENT_DATE.zip
    pushd $KERNEL_OUT/tmp
    zip -r ../../../okernel$LOCALVERSION-$CURRENT_DATE.zip ./system/
    zip -r ../../../okernel$LOCALVERSION-$CURRENT_DATE.zip ./kernel/
fi
