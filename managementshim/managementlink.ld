/* See LICENSE file for the license details of MIT */
/* Copyright 2020 Marno van der Maas */

__mem    = 0x04000000; /*TODO make this dependent on encoding.h */
__stack  = 0x00005000; /*It is important that the stack is never more than 1 page in size*/
__offset = 0x00001000; /*This is the offset between stacks, which in this case is 1 page in size */
__dram   = 0x80000000; /*TODO make this be added by spike itself */

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
