/* See LICENSE file for the license details of MIT */
/* Copyright 2020 Marno van der Maas */

OUTPUT_ARCH( "riscv" )

__mem   = 0x3f00001000; /* TODO make this depend on ENCLAVE_VIRTUAL_ADDRESS_BASE and MAILBOX_SIZE in enclaveLibrary.h */

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
