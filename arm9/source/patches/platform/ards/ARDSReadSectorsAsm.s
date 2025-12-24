#include "asminc.h"

.syntax unified
.thumb

.section "ards_read", "ax"

.global ARDS_readSectorSdhcLabel

@ All the called functions leave every registers unchanged, except for r0 in case the function has a return value

@ARDS_SDReadMultipleSector(u32 sector, u8 * buffer, u32 num_sectors)
BEGIN_ASM_FUNC ARDS_SDReadMultipleSector
    push {r3-r7, lr}
    movs r4, r1
    @ We get the number of bytes total to write, which we'll compare to against in the main loop
    lsls r3, r2, #9

    @ Total written bytes
    movs r5, #0

ARDS_readSectorSdhcLabel:
    @ if not sdhc this needs to be shifted to the left by 9
    lsls r0, #9
    @mov r0, r0

    movs r1, ARDS_SDIO_CMD18_READ_MULTIPLE_BLOCK

    ldr r7, ARDS_SDReadMultipleSector_SpiSendSDIOCommandR0
    bl ARDS_SDReadMultipleSector_Interwork
    bne CMD18_not_ok

read_next_sector:
    ldr r7, ARDS_SDReadMultipleSector_ReadSpiByteTimeout
    bl ARDS_SDReadMultipleSector_Interwork

    cmp r0, ARDS_SPI_START_DATA_TOKEN
    bne wrong_spi_start_token

    @ preload ARDS_ReadSpiByte
    ldr r7, ARDS_SDReadMultipleSector_ReadSpiByte
read_next_byte:
    bl ARDS_SDReadMultipleSector_Interwork
    strb r0, [r4, r5]

    adds r5, #1
    @ Shifting left by 0x17 will set the Zero flag if the number that was shifted is a multiple
    @ of 0x200 (indicating a full sector has been written)
    lsls r0, r5, #0x17
    bne read_next_byte

    @ drop crc
    bl ARDS_SDReadMultipleSector_Interwork
    bl ARDS_SDReadMultipleSector_Interwork

    cmp r3, r5
    bne read_next_sector

    movs r0, #0
    movs r1, ARDS_SDIO_CMD12_STOP_TRANSMISSION
    movs r2, #7

    @ ARDS_SpiSendSDIOCommand is 1 instruction after ARDS_SpiSendSDIOCommandR0
    ldr r7, ARDS_SDReadMultipleSector_SpiSendSDIOCommandR0
    adds r7, #2
    bl ARDS_SDReadMultipleSector_Interwork

    ldr r4, =ARDS_SD_CMD_TIMEOUT_LEN
    ldr r7, ARDS_SDReadMultipleSector_ReadSpiByte
1:
    bl ARDS_SDReadMultipleSector_Interwork
    bne read_timeout_expired
    subs r4, #1
    bne 1b

read_timeout_expired:
    @ movs    r0, r4
    @ subs    r3, r0, #1
    @ sbcs    r0, r3

@ pico loader has no error result
    @ pop     {r3-r7, pc}

CMD18_not_ok:
wrong_spi_start_token:
    @ movs    r0, #0
    pop     {r3-r7, pc}

ARDS_SDReadMultipleSector_Interwork:
    bx r7
.balign 4
.pool

.global ARDS_SDReadMultipleSector_SpiSendSDIOCommandR0
ARDS_SDReadMultipleSector_SpiSendSDIOCommandR0:
    .word 0
.global ARDS_SDReadMultipleSector_ReadSpiByteTimeout
ARDS_SDReadMultipleSector_ReadSpiByteTimeout:
    .word 0
.global ARDS_SDReadMultipleSector_ReadSpiByte
ARDS_SDReadMultipleSector_ReadSpiByte:
    .word 0
