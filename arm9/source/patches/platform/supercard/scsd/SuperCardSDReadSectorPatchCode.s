.cpu arm7tdmi
.syntax unified

#include "SuperCardSDAddresses.h"

.section "scsd_read_sector", "ax"
@ bool scsd_readSector(uint32_t sector, uint8_t *buff, uint32_t readnum)
BEGIN_THUMB_FUNCTION scsd_readSector
    push {r1,r2-r7,lr}
    LOAD_SLOW_EXMEMCNT

    @ r1 for now holds the sector
.global scsd_readSectorSdhcLabel
scsd_readSectorSdhcLabel:
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

    CALL scsd_sdCommandAndDropResponse6 readInterwork

    LOAD_FAST_EXMEMCNT

    @ loads the saved r1 (buff) and r2 (readnum)
    @ into r0 and r1
    pop {r0,r1}
read_sector_loop:
    @ all the functions called in this loop don't change the value of r0 or r1
    @ except scsd_readData, which will increase r0 by 512 automatically
    CALL scsd_readData readInterwork

    subs r1, #1
    bne read_sector_loop

    RELOAD_SLOW_EXMEMCNT

    @ STOP_TRANSMISSION
    SD_COMMAND_ARGUMENT #12
    @ 2nd parameter is passed in r1
    @ and from the loop above r1 is already 0
    LOAD_INTERWORK_FUNCTION sccmn_sdSendClock10 readInterwork r6
    CALL scsd_sdCommandAndDropResponse6 readInterwork

    movs r0, #1
    CALL sccmn_changeMode readInterwork

    RESTORE_EXMEMCNT

    @ returns true
    @ the change mode function above doesn't touch r0, so it's still 1
    @ movs r0, #1
    pop {r3-r7,pc}
INTERWORK readInterwork
INTERWORK_FUNCTION sccmn_changeMode readInterwork
INTERWORK_FUNCTION scsd_sdCommandAndDropResponse6 readInterwork
INTERWORK_FUNCTION scsd_readData readInterwork
INTERWORK_FUNCTION sccmn_sdSendClock10 readInterwork

.balign 4
.pool

.section "scsd_read_data", "ax"

.macro LOAD_U32_ALIGNED_2WORDS srcreg,dstreg,maskreg
    ldmia \srcreg, {r1-r8}
    and    r4, r4, \maskreg
    and    r8, r8, \maskreg
    orr    r4, r4, r2, lsr #16
    orr    r8, r8, r6, lsr #16
    stmia \dstreg!, {r4, r8}
.endm
@ this function will update r0 by incrementing it by 512
@ and will leave r1 unchanged
@ void scsd_readData(void* buffer);
BEGIN_ARM_FUNCTION scsd_readData
    push {r1,r4-r11}
    add r9, r0, #512
    ldr r10,= sd_dataread
waitOnReadTrue_loop:
    ldrh r3, [r10]
    tst r3, #0x100
    bne waitOnReadTrue_loop
    ldr r11,= 0xFFFF0000
read32_loop:
    LOAD_U32_ALIGNED_2WORDS r10, r0, r11
    LOAD_U32_ALIGNED_2WORDS r10, r0, r11
    cmp r0, r9
    blt read32_loop
    @drop crc16
    ldmia r10, {r1-r8}
    @read end transmission bit
    ldrh r1, [r10]
    pop {r1,r4-r11}
    bx lr

.balign 4
.pool
