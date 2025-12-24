/*
    Copyright (C) 2023 lifehackerhansol

    SPDX-License-Identifier: Zlib
*/

#pragma once

#include <libtwl/card/card.h>
#include <cstdint>

// Wrapper for reading from a cartridge.
void cardExt_ReadData(uint64_t command, uint32_t flags, void* buffer, uint32_t length);

// Wrapper for writing to a cartridge.
void cardExt_WriteData(uint64_t command, uint32_t flags, const void* buffer, uint32_t length);

// Wrapper for reading 4 bytes from a cartridge. Usually used for simple commands.
uint32_t cardExt_ReadData4Byte(uint64_t command, uint32_t flags);

// Wrapper for sending a command without an expected return value.
void cardExt_SendCommand(uint64_t command, uint32_t flags);

// Wrapper for writing a byte to SPI and reading back the data.
uint8_t cardExt_ReadWriteSpiByte(uint8_t data);

// Enables the SPI.
static inline void cardExt_EnableSpi(void)
{
    REG_MCCNT0 = (REG_MCCNT0 & ~(MCCNT0_MODE_MASK | MCCNT0_ROM_XFER_IRQ)) | MCCNT0_MODE_SPI | MCCNT0_SPI_HOLD_CS | MCCNT0_ENABLE;
}

// Checks if the cartridge SPI data is ready for receiving.
static inline void cardExt_WaitSpiBusy(void)
{
    while(REG_MCCNT0 & MCCNT0_SPI_BUSY);
}
