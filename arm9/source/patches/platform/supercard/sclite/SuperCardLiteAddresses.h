#define LITE
#include "../asminc.h"

.macro LOAD_EXMEMCNT
    @ loads EXMEMCNT register address
    ldr r7,= 0x04000200
    @ waitstate 4,2 and arm9 slot2 access
    @ r7 holds the EXMEMCNT address, use lower 8 bits as 0
    strb r7, [r7, #4]
.endm

.macro RESTORE_EXMEMCNT
    @ waitstate 4,2 and arm7 slot2 access
    movs r2, #0x80
    strb r2, [r7, #4]
.endm

.macro SD_COMMAND_ARGUMENT value
    movs r2, \value|0x40
.endm

.equ sd_dataadd,     0x9000000
.equ sd_resetaddr,   0x9440000
.equ reg_scsd_cmd,   0x9800000
.equ sd_crc_bit,     0x0100000
.equ sd_rw4,         0x0200000
.equ sd_rw1,         0x0000000
.equ sd_buff_bit,    0x0400000
.equ sd_command_bit, 0x0800000
.equ sd_st,          0x0040000 + sd_crc_bit
.equ sd_status_addr, sd_dataadd + sd_st
.equ sd_buff_bit_addr, sd_dataadd + sd_buff_bit
.equ sd_dataread_4,  sd_dataadd + sd_rw4
.equ sd_dataread_1,  sd_dataadd + sd_rw1
.equ sd_datawrite_4, sd_dataadd + sd_rw4
.equ sd_datawrite_1, sd_dataadd + sd_rw1