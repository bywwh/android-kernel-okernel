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

if [ -f ./new_any_kernel.zip ]
then
    rm ./new_any_kernel.zip
fi

if [ -f ./pvrsrvkm_sgx540_120.ko ]
then
    rm ./pvrsrvkm_sgx540_120.ko
fi

if [ -d ./obj/ ]
then
    rm -r ./obj/
fi

chmod 0644 ./sgx/pvr-source/eurasiacon/build/linux2/tools/cc-check.sh
