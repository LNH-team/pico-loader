#pragma once

/// SDIO commands.
#define SD_CMD0_GO_IDLE_STATE       0
#define SD_CMD2_ALL_SEND_CID        2
#define SD_CMD3_SEND_RELATIVE_ADDR  3
#define SD_CMD7_SELECT_CARD         7
#define SD_CMD8_SEND_IF_COND        8
#define SD_CMD9_SEND_CSD            9
#define SD_CMD12_STOP_TRANSMISSION  12
#define SD_CMD13_SEND_STATUS        13
#define SD_CMD16_SET_BLOCKLEN       16
#define SD_CMD55_APP_CMD            55

#define SD_SPI_CMD58_READ_OCR       58

#define SD_ACMD6_SET_BUS_WIDTH      6
#define SD_ACMD41_SD_SEND_OP_COND   41

/// CMD8 check pattern.
#define SD_IF_COND_PATTERN          0x1AA

/// Response lengths
#define SD_R1_RESPONSE_LENGTH_BITS      48
#define SD_R1_RESPONSE_LENGTH_BYTES     (SD_R1_RESPONSE_LENGTH_BITS / 8)
#define SD_R2_RESPONSE_LENGTH_BITS      136
#define SD_R2_RESPONSE_LENGTH_BYTES     (SD_R2_RESPONSE_LENGTH_BITS / 8)
