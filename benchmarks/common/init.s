# RISC-V baremetal init.s
# This code is executed first.

.section .text.init
entry:
    #TODO make the stack offset dependent on a variable in the linker script
    auipc sp, 0x8           #sp is pc plus 0x8000 which is the stack location
    auipc a0, 0x2           #a0 is the address of the communication page
    call  main              #Call the main function

end:
    csrrw zero, 0x405, zero #This will cause the simulator to exit
    j end                   #Loop when finished if there is no environment to return to.
