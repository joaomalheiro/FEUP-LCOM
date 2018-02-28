#include "rtc_defines.h"

.global _asm_rtc_ih
.global _asm_regC

.data

_asm_regC:
  .byte 0

.text

_asm_rtc_ih:
  xor %eax, %eax
  mov $RTC_REG_C, %al
  outb %al,$RTC_ADDRESS_REG
  xor %eax, %eax
  inb $RTC_DATA_REG, %al
  movb %al, _asm_regC

  ret
