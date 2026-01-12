.cpu arm7tdmi
.syntax unified

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

.section "scsd_sd_command_drop", "ax"
@ void scsd_sdCommandAndDropResponse6(uint32_t dummy, uint32_t argument, SD_COMMANDS command)
@ argument is passed in r1
BEGIN_THUMB_FUNCTION scsd_sdCommandAndDropResponse6
	push {lr}
	cmp r2, #0x52
	@ if command is READ_MULTIPLE_BLOCK, don't send extra clock cycles
	beq 1f
	@ we need to call sccmn_sdSendClock10 after the command is sent, push sccmn_sdSendClock10 and the lr to the stack
	push {r6}
1:
    @ among the pushed registers there are the command and
    @ argument one, which are then used by the crc7 function
    @ and in the loop at the bottom
    @ also allocate an extra 4 bytes for the SD_CRC7 function to put the crc byte
    push {r0-r7}

    @ pass the buffer incremented by 4, so that the crc function
    @ can use a proper indexing method
    add r0, sp, #4
    @ after this call, we get sp back in r1
    bl SD_CRC7

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

    @ the sd command buffer is 6 bytes long
    @ and starts at sp+8 in descending order
    @ r1 holds sp-4, so rather than incrementing it by 3
    @ we decrement it by 1
    subs r2, r1, #1
    movs r0, #5
write_SDCommand_loop:
    ldrb r3, [r2, r0]
    lsls r1, r3, #17
    orrs r3, r1
    lsls r4, r3, #2
    lsls r5, r4, #2
    lsls r6, r5, #2
    stmia r7!, {r3-r6}
    subs r0, #1
    bpl write_SDCommand_loop

    @ drop_response

    @ while(*r7 & 0x01) != 0
SDCommand_drop_resp_nonbusy_loop:
    ldrh r0, [r7]
    lsrs r0, r0, #1
    bcs SDCommand_drop_resp_nonbusy_loop

    movs r6, #4
SDCommand_drop_resp:
    ldmia r7!, {r0-r5}
    subs r6, r6, #1
    bne SDCommand_drop_resp

	@ we pop from the stack r6 as pc, which corresponds to sccmn_sdSendClock10 or lr, if it is sccmn_sdSendClock10 that function
	@ is called, and that function doesn't push a new lr but pops a preexisting one from the stack, which in our case is the
	@ lr provided to this function
    pop {r0-r7,pc}

@ inline uint8_t CRC7_one(uint8_t crcIn, uint8_t data) {
    @ const uint8_t g = 0x89;
    @ uint8_t i;

    @ crcIn ^= data;
    @ for (i = 0; i < 8; i++) {
        @ if (crcIn & 0x80) crcIn ^= g;
        @ crcIn <<= 1;
    @ }

    @ return crcIn;
@ }

@ // Calculate CRC7 value of the buffer
@ // input:
@ //   pBuf - pointer to the buffer
@ // return: the CRC7 value
@ uint32_t CRC7_buf(uint8_t *pBuf) {
    @ uint32_t crc = 0;

    @ for (int i = 4; i >= 0; --i) crc = CRC7_one(crc,pBuf[i]);

    @ return crc << 24;
@ }

@ in r1 puts back the argument it took in in r0
@ SD_CRC7(uint8_t* buff)
SD_CRC7:
    push {r0,r4-r6,lr}
    movs r3, #0
    movs r5, #0x89
    movs r1, #4
    movs r4, #0x80

SD_CRC7_loop:
    movs r2, #8
    ldrb r6, [r0, r1]
    eors r3, r6

CRC7_one_loop:

    @ r4 & 0x80
    tst r3, r4

    beq skip_xor
    eors r3, r5

skip_xor:
    lsls r3, #1
    subs r2, #1
    bne CRC7_one_loop

    subs r1, #1
    bpl SD_CRC7_loop

    @ write at buffer index -1
    strb r3, [r0, r1]
    pop {r1,r4-r6,pc}

.balign 4
.pool

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

.section "scsd_read_data", "ax"

.macro LOAD_U32_ALIGNED_2WORDS srcreg,dstreg,maskreg
    ldmia \srcreg, {r1-r8}
    and	r4, r4, \maskreg
    and	r8, r8, \maskreg
    orr	r4, r4, r2, lsr #16
    orr	r8, r8, r6, lsr #16
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
