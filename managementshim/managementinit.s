# See LICENSE file for the license details of MIT
# Copyright 2020 Marno van der Maas

.section .text.init
entry:
  la    t0, __mem
  la    t1, __stack
  la    t2, __offset
  add   t1, t0, t1
  csrr  t0, mhartid # Get hart id
  mul   t2, t2, t0 # Calculate stack offset for this core's management code
  add   sp, t1, t2
  beq   t0, zero, normal

enclave:
  la    t0, trap
  csrw  mtvec, t0
  call  initialize
  #go to S-mode
  mret
  #jalr  x1, a0, 0
  j     entry

normal:
  call  normalWorld
  la    t0, __dram
  add   sp, x0, x0 # Load zero into sp
  jalr  x0, t0, 0

loop:
  j     loop

  nop # This nop is necessary before trap because the last two bits of the trap address will be cleared when written to mtvec
trap:
  # Make room to save registers
  addi sp, sp, -256

  # Save the registers
  sd ra, 0(sp)
  sd sp, 8(sp)
  sd gp, 16(sp)
  sd tp, 24(sp)
  sd t0, 32(sp)
  sd t1, 40(sp)
  sd t2, 48(sp)
  sd s0, 56(sp)
  sd s1, 64(sp)
  sd a0, 72(sp)
  sd a1, 80(sp)
  sd a2, 88(sp)
  sd a3, 96(sp)
  sd a4, 104(sp)
  sd a5, 112(sp)
  sd a6, 120(sp)
  sd a7, 128(sp)
  sd s2, 136(sp)
  sd s3, 144(sp)
  sd s4, 152(sp)
  sd s5, 160(sp)
  sd s6, 168(sp)
  sd s7, 176(sp)
  sd s8, 184(sp)
  sd s9, 192(sp)
  sd s10, 200(sp)
  sd s11, 208(sp)
  sd t3, 216(sp)
  sd t4, 224(sp)
  sd t5, 232(sp)
  sd t6, 240(sp)

  # Call the C trap handler
  call  handleTrap

  # Check whether enclave is done running
  beqz a0, restore
  j entry

restore:
  # Restore registers
  ld ra, 0(sp)
  ld sp, 8(sp)
  ld gp, 16(sp)
  ld tp, 24(sp)
  ld t0, 32(sp)
  ld t1, 40(sp)
  ld t2, 48(sp)
  ld s0, 56(sp)
  ld s1, 64(sp)
  ld a0, 72(sp)
  ld a1, 80(sp)
  ld a2, 88(sp)
  ld a3, 96(sp)
  ld a4, 104(sp)
  ld a5, 112(sp)
  ld a6, 120(sp)
  ld a7, 128(sp)
  ld s2, 136(sp)
  ld s3, 144(sp)
  ld s4, 152(sp)
  ld s5, 160(sp)
  ld s6, 168(sp)
  ld s7, 176(sp)
  ld s8, 184(sp)
  ld s9, 192(sp)
  ld s10, 200(sp)
  ld s11, 208(sp)
  ld t3, 216(sp)
  ld t4, 224(sp)
  ld t5, 232(sp)
  ld t6, 240(sp)

  # Return to where we were
  addi sp, sp, 256
  mret
