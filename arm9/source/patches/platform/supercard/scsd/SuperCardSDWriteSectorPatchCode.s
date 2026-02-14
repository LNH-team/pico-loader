.cpu arm7tdmi
.syntax unified

#include "SuperCardSDAddresses.h"

.section "scsd_write_sector", "ax"
@ void scsd_writeSector(uint32_t sector, uint8_t* buff, uint32_t writenum)
BEGIN_THUMB_FUNCTION scsd_writeSector
    push {r1,r2,r4-r7,lr}

    @ load EXMEMCNT register into r7
    LOAD_SLOW_EXMEMCNT

    @ r1 for now holds the sector
.global scsd_writeSectorSdhcLabel
scsd_writeSectorSdhcLabel:
    @ if not sdhc this needs to be shifted to the left by 9
    lsls r1, r0, #9
    @ movs r1, r0

    @ r4 sccmn_changeMode
    @ r5 scsd_sdCommandAndDropResponse6
    @ r6 sccmn_sdSendClock10

    adr r3, sccmn_changeMode_writeInterwork_address
    ldmia r3!, {r4-r6}

    @ enable sd access
    @ this function won't touch anything
    movs r0, #3
    @ call sccmn_changeMode
    bl interwork_r4

    @ WRITE_MULTIPLE_BLOCK
    SD_COMMAND_ARGUMENT #25
    @ 2nd parameter is in r1 from above

    @ call scsd_sdCommandAndDropResponse6
    bl interwork_r5

    LOAD_FAST_EXMEMCNT

    @ load the rest of the functions
    @ r4 sccmn_sdio4BitCrc16
    @ r5 scsd_writeData
    ldmia r3!, {r4-r5}

    @ loads the saved r1 (buff) and r2 (writenum)
    @ into r0 and r1
    pop {r0,r1}

write_sector_loop:
    @ all the functions called in this loop don't change the value of r0 and r1
    @ except scsd_writeData, which will increase r0 by 512
    @ sccmn_sdio4BitCrc16 will write the checksum to r2-r3
    @ call sccmn_sdio4BitCrc16, this function will then tail call to r5 (scsd_writeData)
    bl interwork_r4

    subs r1, #1
    bne write_sector_loop

    adr r3, sccmn_changeMode_writeInterwork_address
    @ r4 sccmn_changeMode
    @ r5 scsd_sdCommandAndDropResponse6
    ldmia r3!, {r4-r5}

    RELOAD_SLOW_EXMEMCNT

    @ STOP_TRANSMISSION
    SD_COMMAND_ARGUMENT #12
    @ 2nd parameter is passed in r1
    @ and from the loop above r1 is already 0

    @ call scsd_sdCommandAndDropResponse6
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
INTERWORK_FUNCTION scsd_sdCommandAndDropResponse6 writeInterwork
INTERWORK_FUNCTION sccmn_sdSendClock10 writeInterwork
INTERWORK_FUNCTION sccmn_sdio4BitCrc16 writeInterwork
INTERWORK_FUNCTION scsd_writeData writeInterwork


.section "scsd_write_data", "ax"
.macro WRITE_SINGLE_U16 srcreg, secondreg, dstreg
    lsrs \secondreg, \srcreg, #8
    stmia \dstreg!, {\srcreg,\secondreg}
.endm

.macro WRITE_U32 srcreg,dstreg,maskreg
    lsrs r4, \srcreg, #16
    ands \srcreg, \srcreg, \maskreg
    WRITE_SINGLE_U16 \srcreg, r3, \dstreg
    WRITE_SINGLE_U16 r4, r7, \dstreg
.endm

@ void SCSD_writeBuffer32(uint32_t* buff_u32, uint32_t size)
SCSD_writeBuffer32:
    push {r1,r4-r7,lr}

    adds r7, r0, r1
    mov lr, r7

    @ r5 is SD_DATAADD, taken from the caller
write32_loop:
    ldmia r0!, {r1, r2}
    WRITE_U32 r1, r5, r6
    WRITE_U32 r2, r5, r6
    cmp lr, r0
    bne write32_loop

    pop {r1, r4-r7,pc}

@ void scsd_writeData(void* buffer, int value_to_keep, int crc_buff1, int crc_buff2)
BEGIN_THUMB_FUNCTION scsd_writeData
    @ push sccmn_sdSendClock10 and the current lr reg to the stack, so that at the end we can
    @ call in sequence sccmn_sdSendClock10 and then return
    push {r6}
    push {r1-r7}

    @ loads SD_DATAADD
    movs r5, #0x90
    lsls r5, r5, #20

    @ while(*r5 &0x100) == 0
scsd_writeData_waitOnWriteFalse:
    ldrh r6, [r5]
    lsrs r6, #9
    bcc scsd_writeData_waitOnWriteFalse

    @ dummy read SD_DATAADD
    ldrh r1, [r5]

    @ transmission start bit (lower 16 bit of r5 are 0)
    strh r5, [r5]

    @ mask to use in SCSD_writeBuffer32
    ldr r6,= 0xFFFF

    movs r1, #0x80
    lsls r1, r1, #2
    @ no need for special handling because those 2 functions will be in the same block
    bl SCSD_writeBuffer32
    @ save incremented r0 register
    push {r0}

    movs r1, #8
    @ the pushed crc buffer is at address sp+8
    add r0,sp, #8
    @ no need for special handling because those 2 functions will be in the same block
    bl SCSD_writeBuffer32


    @ write end bit
    movs r3, #0xFF
    strh r3, [r5]

    @ while(*r5 &0x100) != 0
scsd_writeData_waitOnWriteTrue:
    ldrh r6, [r5]
    lsrs r6, #9
    bcs scsd_writeData_waitOnWriteTrue


    @ we pop from the stack r6 as pc, which corresponds to sccmn_sdSendClock10 so we jump to it
    @ sccmn_sdSendClock10 doesn't push a new lr but pops a preexisting one from the stack, which in our case is the
    @ lr provided to this function
    @ also we load in r0 the value stored above
    pop {r0-r7,pc}

.balign 4
.pool
