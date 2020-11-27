# See LICENSE file for the license details of MIT
# Copyright 2020 Marno van der Maas

# RISC-V baremetal init.s
# This code is executed first.

.section .text.init
entry:
    #TODO make the stack offset dependent on a variable in the linker script
    auipc sp, 0x8           #sp is pc plus 0x8000 which is the stack location
    srli  sp, sp, 12        #making sure last 12 bits of sp are 0
    slli  sp, sp, 12
    auipc a0, 0x2           #a0 is the address of the communication page
    srli  a0, a0, 12        #making sure last 12 bits of a0 are 0
    slli  a0, a0, 12
#    mv    s2, x1
    call  main              #Call the main function

end:
    call exit_enclave
    j end
