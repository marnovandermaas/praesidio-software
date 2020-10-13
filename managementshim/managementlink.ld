/* See LICENSE file for the license details of MIT */
/* Copyright 2020 Marno van der Maas */

__mem    = 0x04000000; /*TODO make this dependent on enclaveLibrary.h */
__stack  = 0x00007000; /*TODO make this dependent on the offset in enclaveLibrary.h */
__offset = 0x00001000; /*This is the distance between stacks, which in this case is 1 page in size */
__size   = 0x01000000; /*TODO make this dependent MAX_MANAGEMENT_SHIM_SIZE in enclaveLibrary.h */
__dram   = 0x80000000; /*TODO make this be added by spike itself */
__mpp    = 0x00001800; /*TODO make this dependent on MSTATUS_MPP in encoding.h */

SECTIONS
{
    . = __mem;
    .text : {
        *(.text.init)
        *(.text)
    }

    .data : {
        *(.sbss)
        *(COMMON)
        *(.bss)
        *(.sdata)
        *(.rodata)
        *(.data*)
        *(.got*)
    }
}
