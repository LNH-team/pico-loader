.macro BEGIN_ASM_FUNC name section=text
    .section .\section\().\name\(), "ax", %progbits
    .global \name
    .type \name, %function
    .align 1
\name:
.endm

.macro BEGIN_ASM_FUNC_NO_SECTION name
    .global \name
    .type \name, %function
    .align 1
\name:
.endm


.equ REG_MCCNT0, 0x040001A0
.equ REG_MCD0,   0x040001A2
.equ REG_MCCNT1, 0x040001A4
.equ REG_MCCMD0, 0x040001A8
.equ ARDS_SD_CMD_TIMEOUT_LEN, 0xFFF

.equ ARDS_SDIO_CMD18_READ_MULTIPLE_BLOCK, 18 | 0x40
.equ ARDS_SDIO_CMD12_STOP_TRANSMISSION, 12 | 0x40
.equ ARDS_SDIO_CMD25_WRITE_MULTIPLE_BLOCK, 25 | 0x40
.equ ARDS_SPI_MULTI_BLOCK_WRITE_TOKEN, 0xFC
.equ ARDS_SPI_END_MULTI_BLOCK_WRITE, 0xFD
.equ ARDS_SPI_START_DATA_TOKEN, 0xFE
.equ ARDS_SD_CMD_TIMEOUT_LEN, 0xFFF

.equ ARDS_SD_WRITE_OK, 0x5
.equ ARDS_SD_WRITE_TIMEOUT_LEN, 0xFFFF

.equ ARDS_CMD_F2_SPI_ENABLE, 0xCC
.equ ARDS_CMD_F2_SPI_DISABLE, 0xC8