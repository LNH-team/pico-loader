.cpu arm7tdmi
.syntax unified

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

.section "sclite_sd_command_drop", "ax"
@ void sclite_sdCommandAndDropResponse6(uint32_t dummy, uint32_t argument, SD_COMMANDS command)
@ command is passed in r2
BEGIN_THUMB_FUNCTION sclite_sdCommandAndDropResponse6
    @ among the pushed registers there are the command and
    @ argument one, which are then used at the loop to
    @ send the sd command
    push {r1-r7,lr}

    @ loads reg_scsd_cmd
    movs r7, #0x98
    lsls r7, r7, #20

    @ while(*r7 & 0x01) == 0
SDCommand_loop:
    ldrh r0, [r7]
    lsrs r0, r0, #1
    bcc SDCommand_loop

    @ perform an extra read
    ldrh r0, [r7]

    @ loads sd_buff_bit_addr
    movs r1, #0x94
    lsls r1, r1, #20
    @ lower halfword is 0
    strh r1, [r1]

    @ loads sd_dataadd + sd_command_bit + sd_rw4
    movs r1, #0x9A
    lsls r1, r1, #20

    @ the command buffer starts at sp+4 (r1 and r2) since this loop starts with offset 5
    @ decrement the value of the stack pointer address used by 1
    mov r4,sp
    movs r3, #4
write_SDCommand_loop:
    ldrb r0, [r4, r3]
    lsls r2, r0, #20
    adds r2, r2, r0
    str r2, [r1]
    subs r3, r3, #1
    bpl write_SDCommand_loop

    @ loads sd_dataadd + sd_crc_bit + sd_rw4
    movs r1, #0x9B
    lsls r1, r1, #20
    movs r2, #0
    str r2, [r1]

    @ while(*r7 & 0x01) != 0
SDCommand_loop_2nd:
    ldrh r0, [r7]
    lsrs r0, r0, #1
    bcs SDCommand_loop_2nd

    movs r6, #4
SDCommand_drop_resp:
    ldmia r7!, {r0-r5}
    subs r6, r6, #1
    bne SDCommand_drop_resp

    @ restore stack space
    pop {r1-r7,pc}
.balign 4
.pool

.section "sclite_write_sector", "ax"
@ void sclite_writeSector(uint32_t sector, uint8_t* buff, uint32_t writenum)
BEGIN_THUMB_FUNCTION sclite_writeSector
    push {r1,r2,r4-r7,lr}

    @ load EXMEMCNT register
    LOAD_EXMEMCNT

    @ r1 for now holds the sector
.global sclite_writeSectorSdhcLabel
sclite_writeSectorSdhcLabel:
    @ if not sdhc this needs to be shifted to the left by 9
    lsls r1, r0, #9
    @ movs r1, r0

    @ enable sd access
    @ this function won't touch r1
    movs r0, #3
    CALL sccmn_changeMode writeInterwork

    @ WRITE_MULTIPLE_BLOCK
    SD_COMMAND_ARGUMENT #25
    @ 2nd parameter is in r1 from above

    CALL sclite_sdCommandAndDropResponse6 writeInterwork

    @ this function will trash r4 but leave r0 and r1 untouched
    CALL sccmn_sdSendClock10 writeInterwork

    @ loads the saved r0 and r2 (readnum)
    @ into r0 and r1
    pop {r0,r1}
write_sector_loop:
    @ all the functions called in this loop don't change the value of r0 and r1
    @ except sclite_writeData, which will increase r0 by 512
    @ sccmn_sdio4BitCrc16 will write the checksum to r2-r3
    CALL sccmn_sdio4BitCrc16 writeInterwork

    @ first argument is buffer
    @ regs r2 and r3 hold the crc
    CALL sclite_writeData writeInterwork

    @ this function will trash r4 but leave r0 and r1 untouched
    CALL sccmn_sdSendClock10 writeInterwork

    subs r1, #1
    bne write_sector_loop

    @ STOP_TRANSMISSION
    SD_COMMAND_ARGUMENT #12
    @ 2nd parameter is passed in r1
    @ and from the loop above r1 is already 0

    CALL sclite_sdCommandAndDropResponse6 writeInterwork

    @ this function will trash r4 but leave r0 and r1 untouched
    CALL sccmn_sdSendClock10 writeInterwork

    @ loads sd_dataadd
    movs r0, #0x90
    lsls r0, r0, #20

    @ while(*r1 &0x100) == 0
beginwhile_WriteSector:
    ldrh r1, [r0]
    lsrs r1, #9
    bcc beginwhile_WriteSector

    movs r0, #1
    CALL sccmn_changeMode writeInterwork

    @ restore EXMEMCNT register
    RESTORE_EXMEMCNT

    pop {r4-r7,pc}
INTERWORK writeInterwork
INTERWORK_FUNCTION sclite_writeData writeInterwork
INTERWORK_FUNCTION sccmn_sdio4BitCrc16 writeInterwork
INTERWORK_FUNCTION sccmn_sdSendClock10 writeInterwork
INTERWORK_FUNCTION sccmn_changeMode writeInterwork
INTERWORK_FUNCTION sclite_sdCommandAndDropResponse6 writeInterwork

.balign 4
.pool

.section "sclite_read_sector", "ax"
@ bool sclite_readSector(uint32_t sector, uint8_t *buff, uint32_t readnum)
BEGIN_THUMB_FUNCTION sclite_readSector
    push {r1,r2-r7,lr}
    LOAD_EXMEMCNT

    @ r1 for now holds the sector
.global sclite_readSectorSdhcLabel
sclite_readSectorSdhcLabel:
    @ if not sdhc this needs to be shifted to the left by 9
    lsls r1, r0, #9
    @ movs r1, r0

    @ enable sd access
    @ this function won't touch r1
    movs r0, #3
    CALL sccmn_changeMode readInterwork

    @ READ_MULTIPLE_BLOCK
    SD_COMMAND_ARGUMENT #18
    @ 2nd parameter is in r1 from above

    CALL sclite_sdCommandAndDropResponse6 readInterwork

    @ loads the saved r0 and r2 (writenum)
    @ into r0 and r1
    pop {r0,r1}
read_sector_loop:
    @ all the functions called in this loop don't change the value of r0 or r1
    @ except sclite_readData, which will increase r0 by 512 automatically
    CALL sclite_readData readInterwork

    subs r1, #1
    bne read_sector_loop

    @ STOP_TRANSMISSION
    SD_COMMAND_ARGUMENT #12
    @ 2nd parameter is passed in r1
    @ and from the loop above r1 is already 0

    CALL sclite_sdCommandAndDropResponse6 readInterwork

    @ this function will trash r4 but leave r0 and r1 untouched
    CALL sccmn_sdSendClock10 readInterwork


    movs r0, #1
    CALL sccmn_changeMode readInterwork

    RESTORE_EXMEMCNT

    @ returns true
    @ the change mode function above doesn't touch r0, so it's still 1
    @ movs r0, #1
    pop {r3-r7,pc}
INTERWORK readInterwork
INTERWORK_FUNCTION sccmn_changeMode readInterwork
INTERWORK_FUNCTION sclite_sdCommandAndDropResponse6 readInterwork
INTERWORK_FUNCTION sclite_readData readInterwork
INTERWORK_FUNCTION sccmn_sdSendClock10 readInterwork

.balign 4
.pool

.section "sclite_read_data", "ax"
@ void sclite_readData(void* buffer)
BEGIN_THUMB_FUNCTION sclite_readData
    push {r1,r4-r7,lr}

    @ dummy read SD_STATUS
    ldr r1,= sd_status_addr
    ldrh r1, [r1]

    @ loads sd_buff_bit_addr
    movs r1, #0x94
    lsls r1, r1, #20

    @ loops as long as sd_buff_bit is 1
sclite_readData_buff_bit_loop:
    ldrh r3, [r1]
    lsrs r3, #1
    bcs sclite_readData_buff_bit_loop

    @ loads sd_dataread_4
    movs r1, #0x92
    lsls r1, r1, #20

    @ performs a dummy read of a short to initiate the transfer
    ldrh r2, [r1]

    @ lr holds buffer + 512
    movs r2, #0x80
    lsls r2, r2, #2
    adds r2, r0, r2
    mov lr, r2

    ldmia r1!, {r2-r3}
    stmia r0!, {r2-r3}

sclite_readData_loop:
    @ load 6 ints at the time, for a total of 24 bytes
    ldmia r1!, {r2-r7}
    stmia r0!, {r2-r7}
    cmp	r0,lr
    bne sclite_readData_loop

    @ drop crc16
    ldr r2, [r1]
    ldrh r2, [r1]

    @ loads sd_dataread_1
    movs r1, #0x90
    lsls r1, r1, #20
    ldrh r2, [r1]

    pop {r1,r4-r7,pc}

.balign 4
.pool

.section "sclite_write_data", "ax"
@ void sclite_writeData(void*& buffer, int value_to_keep, int crc_buff1, int crc_buff2)
@ this function updates r0, leaves r1 untouched and thrashes r4
BEGIN_THUMB_FUNCTION sclite_writeData
    @ loads sd_dataadd
    movs r4, #0x90
    lsls r4, r4, #20

    @ save sd_dataadd constant above for later
    push {r1-r7,lr}

waitOnWriteFalse_WriteData:
    ldrh r1, [r4]
    lsrs r1, #9
    bcc waitOnWriteFalse_WriteData

    @ dummy read SD_DATAADD
    ldrh r1, [r4]

    @ transmission start bit (lower 16 bit of r4 are 0)
    strh r4, [r4]


    @ loads sd_datawrite_4
    movs r7, #0x92
    lsls r7, r7, #20

    @ lr holds buffer + 512
    movs r2, #0x80
    lsls r2, r2, #2
    adds r2, r0, r2
    mov lr, r2

    ldmia r0!, {r1-r2}
    stmia r7!, {r1-r2}

sclite_writeData_loop:
    @ load 6 ints at the time, for a total of 24 bytes
    ldmia r0!, {r1-r6}
    stmia r7!, {r1-r6}
    cmp	r0, lr
    bne sclite_writeData_loop


    @ r1 holds a value that has not to be changed
    @ r2-r3 hold the crc value to be written
    @ r4 holds the value sd_dataadd saved at the start
    pop {r1-r4}
    stmia r7!, {r2-r3}

    @ write end bit
    movs r3, #0xff
    strh r3, [r4]

waitOnWriteTrue_WriteData:
    ldrh r3, [r4]
    lsrs r3, #9
    bcs waitOnWriteTrue_WriteData

    @ dummy writes to end the transfer
    movs r3, #0
    str r3, [r4]
    str r3, [r4]

    pop {r5-r7,pc}

.balign 4
.pool
