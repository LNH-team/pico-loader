#define SEND_SDIO_COMMAND_REG r7
#define SEND_COMMAND_REG r10
#define SDIO_CRC_REG r11
#define SEND_WRITE_DATA_ROM_REG r12

.macro BEGIN_ASM_FUNC name
    .global \name
    .type \name, %function
    .align 1
\name:
.endm

.macro CALL_NO_INTERWORK fncreg
	mov lr,pc
	mov pc,\fncreg
.endm

.equ REG_MCCNT0 , 0x040001A0
.equ REG_MCD0   , 0x040001A2
.equ REG_MCCNT1 , 0x040001A4
.equ REG_MCCMD0 , 0x040001A8
.equ REG_MCCMD1 , 0x040001AC
.equ REG_MCSCR0 , 0x040001B0
.equ REG_MCSCR1 , 0x040001B4
.equ REG_MCSCR2 , 0x040001B8
.equ REG_MCD1   , 0x04100010

.equ EZ5H_CMD_SDMC_READ_DATA_LOWER_WORD, 0x0000F7B8
.equ EZ5H_CTRL_READ_0, 0xA0586000
.equ EZ5H_CTRL_READ_512, 0xA1586000
.equ EZ5H_CTRL_READ_4B, 0xA7586000

.macro CHECK_DATA_READY dstreg,srcreg,off,label
	@ read mccnt1 status flag
	ldr \dstreg, [\srcreg, \off]
	@ check that (r2 & (1 << 23)) (data ready), by shifting right 24 bits, if the bit was set, the carry gets updated
	lsrs \dstreg, #24
	bcc \label
.endm
