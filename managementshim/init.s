
.section .text.init
entry:
  la    t0, __mem
  la    t1, __stack
  la    t2, __offset
  add   t1, t0, t1
  csrrs t0, 0x40E, zero
  mul   t2, t2, t0 #Calculate stack offset for this core's management code.
  add   sp, t1, t2
  beq   t0, zero, normal

enclave:
  call  initialize
  jalr  x0, a0, 0

normal:
  call  normalWorld
  la    t0, __dram
  add   sp, x0, x0 #Load zero into sp.
  jalr  x0, t0, 0
