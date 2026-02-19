.cpu arm7tdmi
.syntax unified

#include "SuperCardLiteAddresses.h"

.section "sclite_write_sector", "ax"
@ void sclite_writeSector(uint32_t sector, uint8_t* buff, uint32_t writenum)
BEGIN_THUMB_FUNCTION sclite_writeSector
    push {r1,r2,r4-r7,lr}

    @ load EXMEMCNT register into r7
    LOAD_EXMEMCNT

    @ r1 for now holds the sector
.global sclite_writeSectorSdhcLabel
sclite_writeSectorSdhcLabel:
    @ if not sdhc this needs to be shifted to the left by 9
    lsls r1, r0, #9
    @ movs r1, r0

    @ r4 sccmn_changeMode
    @ r5 sclite_sdCommandAndDropResponse6
    @ r6 sccmn_sdSendClock10

    adr r3, sccmn_changeMode_writeInterworkLite_address
    ldmia r3!, {r4-r6}

    @ enable sd access
    @ this function won't touch anything
    movs r0, #3
    @ call sccmn_changeMode
    bl interwork_r4

    @ WRITE_MULTIPLE_BLOCK
    @ puts the value in r2
    SD_COMMAND_ARGUMENT #25
    @ 2nd parameter is in r1 from above

    @ call sclite_sdCommandAndDropResponse6
    bl interwork_r5

    @ load the rest of the functions
    @ r4 sccmn_sdio4BitCrc16
    @ r5 sclite_writeData
    ldmia r3!, {r4-r5}

    @ loads the saved r1 (buff) and r2 (writenum)
    @ into r0 and r1
    pop {r0,r1}

write_sector_loop:
    @ all the functions called in this loop don't change the value of any register
    @ except sclite_writeData, which will increase r0 by 512
    @ sccmn_sdio4BitCrc16 will write the checksum to r2-r3
    @ call sccmn_sdio4BitCrc16, this function will then tail call to r5 (sclite_writeData)
    bl interwork_r4

    subs r1, #1
    bne write_sector_loop

    adr r3, sccmn_changeMode_writeInterworkLite_address
    @ r4 sccmn_changeMode
    @ r5 sclite_sdCommandAndDropResponse6
    ldmia r3!, {r4-r5}

    @ STOP_TRANSMISSION
    @ puts the value in r2
    SD_COMMAND_ARGUMENT #12
    @ 2nd parameter is passed in r1
    @ and from the loop above r1 is already 0

    @ call sclite_sdCommandAndDropResponse6
    bl interwork_r5

    @ loads sd_dataadd
    movs r0, #0x90
    lsls r0, r0, #20

    @ while(*r0 &0x100) == 0
beginwhile_WriteSector:
    ldrh r1, [r0]
    lsrs r1, #9
    bcc beginwhile_WriteSector

    movs r0, #1
    @ call sccmn_changeMode writeInterwork
    bl interwork_r4

    @ restore EXMEMCNT register
    RESTORE_EXMEMCNT

    pop {r4-r7,pc}
interwork_r4:
    bx r4
interwork_r5:
    bx r5
.balign 4
.pool
INTERWORK_FUNCTION sccmn_changeMode writeInterwork
INTERWORK_FUNCTION sclite_sdCommandAndDropResponse6 writeInterwork
INTERWORK_FUNCTION sccmn_sdSendClock10 writeInterwork
INTERWORK_FUNCTION sccmn_sdio4BitCrc16 writeInterwork
INTERWORK_FUNCTION sclite_writeData writeInterwork

.section "sclite_write_data", "ax"
@ void sclite_writeData(void*& buffer, int value_to_keep, int crc_buff1, int crc_buff2)
@ this function updates r0 and leaves everything else untouched
@ in r6 is the pointer to sccmn_sdSendClock10
BEGIN_THUMB_FUNCTION sclite_writeData
    @ push sccmn_sdSendClock10 and the current lr reg to the stack, so that at the end we can
    @ call in sequence sccmn_sdSendClock10 and then return
    push {r6}
    push {r1-r7}
    @ loads sd_dataadd
    movs r4, #0x90
    lsls r4, r4, #20

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
    cmp    r0, lr
    bne sclite_writeData_loop


    @ r1 holds a value that has not to be changed
    @ r2-r3 hold the crc value to be written
    pop {r1-r3}

    stmia r7!, {r2-r3}
    @ loads sd_dataadd
    movs r4, #0x90
    lsls r4, r4, #20

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


    @ we pop from the stack r6 as pc, which corresponds to sccmn_sdSendClock10 so we jump to it
    @ sccmn_sdSendClock10 doesn't push a new lr but pops a preexisting one from the stack, which in our case is the
    @ lr provided to this function
    pop {r4-r7,pc}

.balign 4
.pool
