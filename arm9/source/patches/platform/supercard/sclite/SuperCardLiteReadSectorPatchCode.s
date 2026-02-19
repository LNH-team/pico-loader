.cpu arm7tdmi
.syntax unified

#include "SuperCardLiteAddresses.h"

.section "sclite_read_sector", "ax"
@ bool sclite_readSector(uint32_t sector, uint8_t *buff, uint32_t readnum)
BEGIN_THUMB_FUNCTION sclite_readSector
    push {r1,r2-r7,lr}

    @ load EXMEMCNT register into r7
    LOAD_EXMEMCNT

    @ r1 for now holds the sector
.global sclite_readSectorSdhcLabel
sclite_readSectorSdhcLabel:
    @ if not sdhc this needs to be shifted to the left by 9
    lsls r1, r0, #9
    @ movs r1, r0

    @ enable sd access
    @ this function won't touch anything
    movs r0, #3
    CALL sccmn_changeMode readInterwork

    @ READ_MULTIPLE_BLOCK
    @ puts the value in r2
    SD_COMMAND_ARGUMENT #18
    @ 2nd parameter is in r1 from above

    CALL sclite_sdCommandAndDropResponse6 readInterwork

    @ loads the saved r1 (buff) and r2 (readnum)
    @ into r0 and r1
    pop {r0,r1}
read_sector_loop:
    @ all the functions called in this loop don't change the value of any register
    @ except sclite_readData, which will increase r0 by 512 automatically
    CALL sclite_readData readInterwork

    subs r1, #1
    bne read_sector_loop

    @ STOP_TRANSMISSION
    @ puts the value in r2
    SD_COMMAND_ARGUMENT #12
    @ 2nd parameter is passed in r1
    @ and from the loop above r1 is already 0
    LOAD_INTERWORK_FUNCTION sccmn_sdSendClock10 readInterwork r6
    CALL sclite_sdCommandAndDropResponse6 readInterwork

    @ this function won't touch anything
    @ CALL sccmn_sdSendClock10 readInterwork


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

    @ loads sd_dataread
    movs r1, #0x91
    lsls r1, r1, #20

    @ loops as long as sd_dataread is 0x100
sclite_readData_buff_bit_loop:
    ldrh r3, [r1]
    lsrs r3, #9
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
    cmp    r0,lr
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
