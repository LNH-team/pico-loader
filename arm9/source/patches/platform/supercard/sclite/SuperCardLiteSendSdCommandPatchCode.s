.cpu arm7tdmi
.syntax unified

#include "SuperCardLiteAddresses.h"

.section "sclite_sd_command_drop", "ax"
@ void sclite_sdCommandAndDropResponse6(uint32_t dummy, uint32_t argument, SD_COMMANDS command)
@ command is passed in r2, in r6 is the pointer to sccmn_sdSendClock10
BEGIN_THUMB_FUNCTION sclite_sdCommandAndDropResponse6
    push {lr}
    cmp r2, #0x52
    @ if command is READ_MULTIPLE_BLOCK, don't send extra clock cycles
    beq 1f
    @ we need to call sccmn_sdSendClock10 after the command is sent, push sccmn_sdSendClock10 and the lr to the stack
    push {r6}
1:
    @ among the pushed registers there are the command and
    @ argument one, which are then used at the loop to
    @ send the sd command
    push {r1-r7}

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

    @ we pop from the stack r6 as pc, which corresponds to sccmn_sdSendClock10 or lr, if it is sccmn_sdSendClock10 that function
    @ is called, and that function doesn't push a new lr but pops a preexisting one from the stack, which in our case is the
    @ lr provided to this function
    pop {r1-r7,pc}

.balign 4
.pool
