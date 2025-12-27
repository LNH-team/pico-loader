#include "asminc.h"

.syntax unified
.thumb

.section "datel_cycle_spi", "ax"
@ All the functions leave every registers unchanged, except for r0 in case the function has a return value

@void DATEL_CycleSpi();
BEGIN_ASM_FUNC DATEL_CycleSpi
    push {r0-r4, lr}
	adr r0, DATEL_CycleSpi_data
	ldm r0!, {r1,r3,r4}
    strh r3, [r1] @ Enable Spi
    movs r0, DATEL_CMD_F2_SPI_ENABLE
	
	@ DATEL_SendNtrCommandF2
    movs r2, #0xF2
    lsls r0, r0, #8
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
	
    strh r3, [r1] @ Enable Spi
    pop {r0-r4, pc}
.balign 4
.pool
DATEL_CycleSpi_data:
	.word REG_MCCNT0
	.word 0x0000A040 @ MCCNT0_MODE_SPI | MCCNT0_SPI_HOLD_CS | MCCNT0_ENABLE
	.word 0xA07F6000 @ MCCNT1_RESET_OFF | MCCNT1_CMD_SCRAMBLE | MCCNT1_READ_DATA_DESCRAMBLE | MCCNT1_CLOCK_SCRAMBLER | MCCNT1_LATENCY2(0x3F)

.section "datel_read_spi", "ax"
@ NOTE!!!: This function needs to set r0 last with mov or something similar so that it updates the zero flags
@u8 DATEL_ReadSpiByte(void);
BEGIN_ASM_FUNC DATEL_ReadSpiByte
    movs r0, 0xFF
@u8 DATEL_ReadWriteSpiByte(u8);
BEGIN_ASM_FUNC DATEL_ReadWriteSpiByte
    push {r1-r3, lr}
    ldr r3, =REG_MCCNT0
    strh r0, [r3, #2]
1:
    ldrh r1, [r3]
    lsrs r1, r1, #8
    bcs 1b
    @uppper half always 0
    ldrh r0, [r3, #2]
    cmp r0, #0
    pop {r1-r3, pc}

@u8 DATEL_ReadSpiByteTimeout(void);
BEGIN_ASM_FUNC DATEL_ReadSpiByteTimeout
    push {r1-r4, lr}
    @ use a timeout of 0x1000 instead of 0xFFF, easier to setup
    @ ldr r4, =DATEL_SD_CMD_TIMEOUT_LEN
    movs r4, #1
    lsls r4, #12
1:
    bl DATEL_ReadSpiByte
    cmp r0, #0xFF
    bne 1f
    subs r4, r4, #1
    bne 1b
1:
    pop {r1-r4, pc}

@bool DATEL_WaitSpiByteTimeout();
BEGIN_ASM_FUNC DATEL_WaitSpiByteTimeout
    push {r1-r4, lr}
    @ use a timeout of 0x1000 instead of 0xFFFF, easier to setup
    @ ldr r2, =DATEL_SD_WRITE_TIMEOUT_LEN
    movs r2, #1
    lsls r2, #16
1:
    bl DATEL_ReadSpiByte
    bne 1f
    subs r2, #1
    bne 1b

    movs r0, #0
    pop {r1-r4, pc}

1:
    movs r0, #1
    pop {r1-r4, pc}

.section "datel_spi_send", "ax"
@ NOTE!!!: This function needs to set r0 last with mov or something similar so that it updates the zero flags
@u8 DATEL_SpiSendSDIOCommandR0(u32 arg, u8 cmd);
BEGIN_ASM_FUNC DATEL_SpiSendSDIOCommandR0
    movs r2, 0
@u8 DATEL_SpiSendSDIOCommand(u32 arg, u8 cmdId, int extraBytes);
BEGIN_ASM_FUNC DATEL_SpiSendSDIOCommand
    push {r0-r7, lr}

    adr r4, DATEL_SpiSendSDIOCommandR0_ReadWriteSpiByte
    @ r1 contains ReadWriteSpiByte
    @ r3 contains ReadSpiByteTimeout
    @ r7 contains CycleSpi, one shot
    ldm r4!, {r1,r3,r7}

    @ we use the cmd and arg directly from the stack
    @ r0 is on top, r1 is right below, we read the command id as the last byte pushed of r1,
    @ so at sp -1, the command arguments are at sp+0,sp+1,sp+2,sp+3, the 6th byte is garbage data read
    @ from sp+4, we don't need it to be something meaningful
    mov r4, sp
    @ TODO: this could maybe be optimized by setting 4 in r5, and having the last extra byte be sent by the timeout function itself
    subs r4, #1

    @ branch to DATEL_CycleSpi
    bl DATEL_SpiSendSDIOCommandR0_Interwork
    movs r5, #5

    @ preload DATEL_ReadWriteSpiByte
    movs r7, r1
1:
    ldrb r0, [r4, r5]
    @ branch to DATEL_ReadWriteSpiByte
    bl DATEL_SpiSendSDIOCommandR0_Interwork
    subs r5, r5, #1
    bcs 1b
    @ branch to DATEL_ReadSpiByteTimeout
    movs r7, r3
    bl DATEL_SpiSendSDIOCommandR0_Interwork

    @ load DATEL_ReadSpiByte since it's 1 thumb instruction before DATEL_ReadWriteSpiByte
    subs r7, r1, #2

    @ r5 is -1 from the iteration above
    @ movs r5, #0
    movs r6, r0
1:
    subs r2, #1
    bcc 2f
    movs r0, r6
    pop {r1}
    pop {r1-r7, pc}
2:
    @ branch to DATEL_ReadSpiByte
    bl DATEL_SpiSendSDIOCommandR0_Interwork
    b 1b

DATEL_SpiSendSDIOCommandR0_Interwork:
    bx r7
.balign 4
.pool

.global DATEL_SpiSendSDIOCommandR0_ReadWriteSpiByte
DATEL_SpiSendSDIOCommandR0_ReadWriteSpiByte:
    .word 0
.global DATEL_SpiSendSDIOCommandR0_ReadSpiByteTimeout
DATEL_SpiSendSDIOCommandR0_ReadSpiByteTimeout:
    .word 0
.global DATEL_SpiSendSDIOCommandR0_CycleSpi
DATEL_SpiSendSDIOCommandR0_CycleSpi:
    .word 0
