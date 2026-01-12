#include "asminc.h"

.syntax unified
.thumb

.section "datel_read_spi", "ax"
@ NOTE!!!: This function needs to set r0 last with mov or something similar so that it updates the zero flags
@u8 datel_readSpiByte(void);
BEGIN_ASM_FUNC datel_readSpiByte
    movs r0, 0xFF
@u8 datel_readWriteSpiByte(u8);
BEGIN_ASM_FUNC datel_readWriteSpiByte
    push {r1-r3}
    ldr r3, =REG_MCCNT0
    @ Wait if there's a transfer in progress (can happen if the random byte sent by the cycle spi function is still on its way)
1:
    ldrh r1, [r3]
    lsrs r1, r1, #8
    bcs 1b

    @ Actually write the byte of interest and wait for it to be sent
    strh r0, [r3, #2]
1:
    ldrh r1, [r3]
    lsrs r1, r1, #8
    bcs 1b
    @uppper half always 0
    ldrh r0, [r3, #2]
    cmp r0, #0
    pop {r1-r3}
    mov pc,lr

@u16 datel_readWriteSpiShort();
BEGIN_ASM_FUNC datel_readSpiShort
    push {r1-r7, lr}
    bl datel_readSpiByte
    movs r1, r0
    bl datel_readSpiByte
    lsls r0, #8
    orrs r0, r1
    pop {r1-r7, pc}

@u8 datel_readSpiByteTimeout(void);
BEGIN_ASM_FUNC datel_readSpiByteTimeout
    push {r1-r4, lr}
    @ use a timeout of 0x1000 instead of 0xFFF, easier to setup
    @ ldr r4, =DATEL_SD_CMD_TIMEOUT_LEN
    movs r4, #1
    lsls r4, #12
1:
    bl datel_readSpiByte
    cmp r0, #0xFF
    bne 1f
    subs r4, r4, #1
    bne 1b
1:
    pop {r1-r4,pc}

@bool datel_waitSpiByteTimeout();
BEGIN_ASM_FUNC datel_waitSpiByteTimeout
    push {r1-r4}
    push {lr}
    @ use a timeout of 0x10000 instead of 0xFFFF, easier to setup
    @ ldr r2, =DATEL_SD_WRITE_TIMEOUT_LEN
    movs r2, #1
    lsls r2, #16
1:
    bl datel_readSpiByte
    bne 1f
    subs r2, #1
    bne 1b

    movs r0, #0
    b end

1:
    movs r0, #1
end:
    pop {r1}
    mov lr,r1
    pop {r1-r4}
    mov pc,lr

.section "datel_cycle_spi", "ax"
@void datel_cycleSpi();
BEGIN_ASM_FUNC datel_cycleSpi
    push {r0-r5, lr}
    adr r0, datel_cycleSpi_data
    ldm r0!, {r1,r3,r4}
    @ First send spi disable command, the second time this loop is called we send spi enable
    movs r5, DATEL_CMD_F2_SPI_DISABLE
datel_sendNtrCommandF2:
    movs r2, #0xF2
    lsls r0, r5, #8
    str r2, [r1, #8]
    str r0, [r1, #12]

    @ we shift the 0xF2 loaded above by 14 to get 0xXXXX8000 (MCCNT0_ENABLE)
    lsls r2, #14
    strh r2, [r1]
    @ REG_MCCNT0 + 4 = REG_MCCNT1
    str r4, [r1, #4]
1:
    ldr r2, [r1, #4]
    cmp r2, #0
    blt 1b

    adds r5, #4
    cmp r5, DATEL_CMD_F2_SPI_ENABLE
    @ Enable spi and also send a dummy byte at the same time by writing first to REG_MCCNT0 and then to REG_MCD0
    str r3, [r1]
    beq datel_sendNtrCommandF2

    pop {r0-r5, pc}
.balign 4
.pool
datel_cycleSpi_data:
    .word REG_MCCNT0
    .word 0x00FFA040 @ MCCNT0_MODE_SPI | MCCNT0_SPI_HOLD_CS | MCCNT0_ENABLE in lower 16 bit, 0xFF in upper 16
    .word 0xA07F6000 @ MCCNT1_RESET_OFF | MCCNT1_CMD_SCRAMBLE | MCCNT1_READ_DATA_DESCRAMBLE | MCCNT1_CLOCK_SCRAMBLER | MCCNT1_LATENCY2(0x3F)

.section "datel_spi_send", "ax"
@ NOTE!!!: This function needs to set r0 last with mov or something similar so that it updates the zero flags
@u8 datel_spiSendSDIOCommandR0(u32 arg, u8 cmd);
BEGIN_ASM_FUNC datel_spiSendSDIOCommandR0
    movs r2, 0
@u8 datel_spiSendSDIOCommand(u32 arg, u8 cmdId, int extraBytes);
BEGIN_ASM_FUNC datel_spiSendSDIOCommand
    push {r0-r7, lr}

    ldr r3, datel_spiSendSDIOCommandR0_CycleSpi
    @ branch to datel_cycleSpi
    bl datel_spiSendSDIOCommandR0_InterworkR3

    adr r3, datel_spiSendSDIOCommandR0_ReadSpiByteTimeout
    @ r3 contains ReadSpiByteTimeout
    @ r7 contains ReadWriteSpiByte
    ldm r3, {r3,r7}

    @ we use the cmd and arg directly from the stack
    @ r0 is on top, r1 is right below, we read the command id as the last byte pushed of r1,
    @ so at sp -1, the command arguments are at sp+0,sp+1,sp+2,sp+3, the 6th byte is garbage data read
    @ from sp+4, we don't need it to be something meaningful
    mov r4, sp
    @ TODO: this could maybe be optimized by setting 4 in r5, and having the last extra byte be sent by the timeout function itself
    subs r4, #1

    movs r5, #6

    @ sets up lr so that the interwork function jumps back here
    bl 1f
1:
    subs r5, r5, #1
    ldrb r0, [r4, r5]
    @ branch to datel_readWriteSpiByte
    bcs datel_spiSendSDIOCommandR0_Interwork

    @ branch to datel_readSpiByteTimeout
    bl datel_spiSendSDIOCommandR0_InterworkR3

    @ load datel_readSpiByte since it's 1 thumb instruction before datel_readWriteSpiByte
    subs r7, r7, #2

    @ Save the return value
    movs r6, r0

    @ sets up lr so that the interwork function jumps back here
    bl 1f
1:
    subs r2, #1
    @ branch to datel_readSpiByte
    bcc datel_spiSendSDIOCommandR0_Interwork
    movs r0, r6
    pop {r1}
    pop {r1-r7}
    pop {r2}
    mov pc,r2

datel_spiSendSDIOCommandR0_Interwork:
    bx r7
datel_spiSendSDIOCommandR0_InterworkR3:
    bx r3
.balign 4
.pool
.global datel_spiSendSDIOCommandR0_CycleSpi
datel_spiSendSDIOCommandR0_CycleSpi:
    .word 0
.global datel_spiSendSDIOCommandR0_ReadSpiByteTimeout
datel_spiSendSDIOCommandR0_ReadSpiByteTimeout:
    .word 0
.global datel_spiSendSDIOCommandR0_ReadWriteSpiByte
datel_spiSendSDIOCommandR0_ReadWriteSpiByte:
    .word 0
