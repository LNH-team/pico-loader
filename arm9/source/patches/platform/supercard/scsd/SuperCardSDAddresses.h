#include "../asminc.h"

.macro SD_COMMAND_ARGUMENT value
    movs r2, \value|0x40
.endm

.macro LOAD_SLOW_EXMEMCNT
    @ loads EXMEMCNT register address
    ldr r7,= 0x04000200
    @ waitstate 4,2 and arm9 slot2 access
    @ r7 holds the EXMEMCNT address, use lower 8 bits as 0
    strb r7, [r7, #4]
.endm

.macro RELOAD_SLOW_EXMEMCNT
    @ waitstate 4,2 and arm9 slot2 access
    @ r7 holds the EXMEMCNT address, use lower 8 bits as 0
    strb r7, [r7, #4]
.endm

.macro LOAD_FAST_EXMEMCNT
    @ loads EXMEMCNT register address
    movs r0, #0x18
    @ waitstate 2,1 and arm9 slot2 access
    strb r0, [r7, #4]
.endm

.macro RESTORE_EXMEMCNT
    @ waitstate 4,2 and arm7 slot2 access
    movs r2, #0x80
    strb r2, [r7, #4]
.endm

.equ sd_dataadd,     0x9000000
.equ sd_dataread,    0x9100000
.equ sd_resetaddr,   0x9440000
.equ reg_scsd_cmd,   0x9800000