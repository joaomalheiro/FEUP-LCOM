#include "i8042.h"

.global _asm_readbyte
.global _asm_mouse_errorlevel

.data

_asm_readbyte:
  .byte 0

_asm_mouse_errorlevel:
  .byte 0

.global _asm_readpacketbyte

.text

_asm_readpacketbyte:
  xor %eax,%eax
  inb $KBC_STATUS_REG, %al          #reading the status register, saving to al
  testb $KBC_OBF, %al               #testing if the OBF flag is set, i.e if there is data available to read
  jz datanotavailable
  testb $KBC_AUX, %al
  jz no_data
  testb $KBC_PARITY, %al            #testing if there is a parity error, data is invalid if so
  jnz parityerror
  testb $KBC_TIMEOUT, %al           #testing if there is a timeout error, data is invalid if so
  jnz timeouterror
  /*if none of the errors ocurred, the data is valid and is ready to read */
  inb $KBC_OUT_BUF, %al
  movb %al, _asm_readbyte           #returning the read byte through a global variable
  movb $0, _asm_mouse_errorlevel          #there were no errors, errorlevel = 0
  jmp end

datanotavailable:
  #Data is not available to read, return error level = -1
  movb $1, _asm_mouse_errorlevel
  jmp end

no_data :
  movb $1, _asm_mouse_errorlevel
  jmp end


parityerror:
  #Data has parity error, return error level = -2
  movb $-2, _asm_mouse_errorlevel
  jmp end

timeouterror:
  #Data has timeout error, return error level = -3
  movb $-3, _asm_mouse_errorlevel
  jmp end



end:
  ret
