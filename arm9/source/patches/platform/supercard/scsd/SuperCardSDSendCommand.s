.cpu arm7tdmi
.syntax unified

#include "SuperCardSDAddresses.h"

.section "scsd_sd_command_drop", "ax"
@ void scsd_sdCommandAndDropResponse6(uint32_t dummy, uint32_t argument, SD_COMMANDS command)
@ command is passed in r2, in r6 is the pointer to sccmn_sdSendClock10
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
