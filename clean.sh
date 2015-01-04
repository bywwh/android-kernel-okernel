#!/bin/bash
###############################################################################
#
#                           Kernel Build Script
#
###############################################################################
# 2014-09-23 Shev_t       : modified
# 2011-10-24 effectivesky : modified
# 2010-12-29 allydrop     : created
###############################################################################
make O=./obj/KERNEL_OBJ/ clean

if [ -f ./zImage ]
then
    rm ./zImage
fi

if [ -f ./okernel*.zip ]
then
    rm ./okernel*.zip
fi

if [ -d ./obj/ ]
then
    rm -r ./obj/
fi
