/* See LICENSE file for the license details of MIT */
/* Copyright 2020 Marno van der Maas */

OUTPUT_ARCH( "riscv" )

__mem   = 0x00000000;

SECTIONS
{
    . = __mem;

    /* code segment */
    .text : {
        *(.text.init) /* add the instruction from init.s */
        *(.text) /* add the instructions compiled from enclave.c */
    }

    /* data segment */
    .data : { *(.data) }
}
