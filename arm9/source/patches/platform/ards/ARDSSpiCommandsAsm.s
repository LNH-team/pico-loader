#include "asminc.h"

.syntax unified
.thumb

.section "ards_ntr_command", "ax"
@ All the functions leave every registers unchanged, except for r0 in case the function has a return value

.equ REG_MCCNT0, 0x040001A0
.equ REG_MCD0,   0x040001A2
.equ REG_MCCNT1, 0x040001A4
.equ REG_MCCMD0, 0x040001A8
.equ ARDS_SD_CMD_TIMEOUT_LEN, 0xFFF

@void ARDS_SendNtrCommandF2(u8 param2);
BEGIN_ASM_FUNC ARDS_SendNtrCommandF2
	push    {r0-r3,lr}
	movs    r2, #0xF2
	ldr     r1, =REG_MCCNT0
	@ loads REG_MCCMD0 in r1, we do this so that the only loaded constant is REG_MCCNT0, shared with other pieces of code
	lsls    r3, r0, #8
	str     r2, [r1, #8]
	str     r3, [r1, #12]

	@ we shift 0xF2 above by 14 to get 0xXXXX8000
	lsls    r2, #14

	ldrh    r3, [r1]
	lsls    r3, r3, #19
	lsrs    r3, r3, #19
	orrs    r3, r2
	strh    r3, [r1]
	ldr     r2, =#0xA07F6000
	@ REG_MCCNT0 + 4 = REG_MCCNT1
	str     r2, [r1, #4]
1:
	ldr     r2, [r1, #4]
	cmp     r2, #0
	blt     1b
	pop     {r0-r3,pc}

@void ARDS_EnableSpi();
BEGIN_ASM_FUNC ARDS_EnableSpi
	push    {r0-r3,lr}
	ldr     r1, =REG_MCCNT0
	@ Use 0x1FBF here
	ldr     r3, =#0xA0401FBF
	ldrh    r2, [r1]
	ands    r2, r3
	@ Use 0xA040 here
	lsrs    r3, #16
	orrs    r3, r2
	strh    r3, [r1]
	pop     {r0-r3,pc}

@void ARDS_CycleSpi();
BEGIN_ASM_FUNC ARDS_CycleSpi
	push    {r0-r3, lr}
	movs    r0, ARDS_CMD_F2_SPI_DISABLE
	bl      ARDS_SendNtrCommandF2
	bl      ARDS_EnableSpi
	ldr     r1, ARDS_CycleSpi_ReadSpiByte
	bl      ARDS_CycleSpi_Interwork
	movs    r0, ARDS_CMD_F2_SPI_ENABLE
	bl      ARDS_SendNtrCommandF2
	bl      ARDS_EnableSpi
	pop     {r0-r3, pc}
ARDS_CycleSpi_Interwork:
	bx r1
.balign 4
.pool
.global ARDS_CycleSpi_ReadSpiByte
ARDS_CycleSpi_ReadSpiByte:
	.word 0

.section "ards_read_spi", "ax"
@ NOTE!!!: This function needs to set r0 last with mov or something similar so that it updates the zero flags
@u8 ARDS_ReadSpiByte(void);
BEGIN_ASM_FUNC ARDS_ReadSpiByte
    movs r0, 0xFF
@u8 ARDS_ReadWriteSpiByte(u8);
BEGIN_ASM_FUNC ARDS_ReadWriteSpiByte
	push    {r1-r3, lr}
	movs    r2, #0x80
	ldr     r3, =REG_MCCNT0
	strh    r0, [r3, #2]
1:
	ldrh    r1, [r3]
	tst     r1, r2
	bne     1b
	@uppper half always 0
	ldrh    r0, [r3, #2]
	cmp     r0, #0
	pop     {r1-r3, pc}

@u8 ARDS_ReadSpiByteTimeout(void);
BEGIN_ASM_FUNC ARDS_ReadSpiByteTimeout
	push    {r1-r4, lr}
	@ use a timeout of 0x1000 instead of 0xFFF, easier to setup
	@ ldr     r4, =ARDS_SD_CMD_TIMEOUT_LEN
	movs    r4, #1
	lsls    r4, #12
1:
	bl      ARDS_ReadSpiByte
	cmp     r0, #0xFF
	bne     1f
	subs    r4, r4, #1
	bne     1b
1:
	pop     {r1-r4, pc}

@bool ARDS_WaitSpiByteTimeout();
BEGIN_ASM_FUNC ARDS_WaitSpiByteTimeout
	push    {r1-r4, lr}
	@ use a timeout of 0x1000 instead of 0xFFFF, easier to setup
	@ ldr     r2, =ARDS_SD_WRITE_TIMEOUT_LEN
	movs    r2, #1
	lsls    r2, #16
1:
	bl      ARDS_ReadSpiByte
	bne     1f
	subs    r2, #1
	bne     1b

	movs    r0, #0
	pop     {r1-r4, pc}

1:
	movs    r0, #1
	pop     {r1-r4, pc}

.section "ards_spi_send", "ax"
@ NOTE!!!: This function needs to set r0 last with mov or something similar so that it updates the zero flags
@u8 ARDS_SpiSendSDIOCommandR0(u32 arg, u8 cmd);
BEGIN_ASM_FUNC ARDS_SpiSendSDIOCommandR0
	movs r2, 0
@u8 ARDS_SpiSendSDIOCommand(u32 arg, u8 cmdId, int extraBytes);
BEGIN_ASM_FUNC ARDS_SpiSendSDIOCommand
	push    {r0-r7, lr}
	
	adr r4, ARDS_SpiSendSDIOCommandR0_ReadWriteSpiByte
	@ r1 contains ReadWriteSpiByte
	@ r3 contains ReadSpiByteTimeout
	@ r7 contains CycleSpi, one shot
	ldm r4!, {r1,r3,r7}
	
	@ we use the cmd and arg directly from the stack
	@ r0 is on top, r1 is right below, we read the command id as the last byte pushed of r1,
	@ so at sp -1, the command arguments are at sp+0,sp+1,sp+2,sp+3, the 6th byte is garbage data read
	@ from sp+4, we don't need it to be something meaningful
	mov     r4, sp
	@ TODO: this could maybe be optimized by setting 4 in r5, and having the last extra byte be sent by the timeout function itself
	subs    r4, #1
	
	@ branch to ARDS_CycleSpi
	bl      ARDS_SpiSendSDIOCommandR0_Interwork
	movs    r5, #5

	@ preload ARDS_ReadWriteSpiByte
	movs    r7, r1
1:
	ldrb    r0, [r4, r5]
	@ branch to ARDS_ReadWriteSpiByte
	bl      ARDS_SpiSendSDIOCommandR0_Interwork
	subs    r5, r5, #1
	bcs     1b
	@ branch to ARDS_ReadSpiByteTimeout
	movs    r7, r3
	bl      ARDS_SpiSendSDIOCommandR0_Interwork
	
	@ load ARDS_ReadSpiByte since it's 1 thumb instruction before ARDS_ReadWriteSpiByte
	subs r7, r1, #2
	
	@ r5 is -1 from the iteration above
	@ movs    r5, #0
	movs    r6, r0
1:
	subs    r2, #1
	bcc     2f
	movs    r0, r6
	pop     {r1}
	pop     {r1-r7, pc}
2:
	@ branch to ARDS_ReadSpiByte
	bl      ARDS_SpiSendSDIOCommandR0_Interwork
	b       1b

ARDS_SpiSendSDIOCommandR0_Interwork:
	bx r7
.balign 4
.pool

.global ARDS_SpiSendSDIOCommandR0_ReadWriteSpiByte
ARDS_SpiSendSDIOCommandR0_ReadWriteSpiByte:
	.word 0
.global ARDS_SpiSendSDIOCommandR0_ReadSpiByteTimeout
ARDS_SpiSendSDIOCommandR0_ReadSpiByteTimeout:
	.word 0
.global ARDS_SpiSendSDIOCommandR0_CycleSpi
ARDS_SpiSendSDIOCommandR0_CycleSpi:
	.word 0
