/*
    Copyright (C) 2023 lifehackerhansol

    SPDX-License-Identifier: Zlib
*/

#include <nds/ndstypes.h>
#include <libtwl/card/card.h>
#include "libtwl_ext.h"

void cardExt_ReadData(uint64_t command, uint32_t flags, void* buffer, uint32_t length) {
    card_romSetCmd(command);
    card_romStartXfer(flags, false);
    if ((uint32_t)buffer & 3)
        card_romCpuReadUnaligned((uint8_t*)buffer, length);
    else
        card_romCpuRead((u32*)buffer, length);
}

void cardExt_WriteData(uint64_t command, uint32_t flags, const void* buffer, uint32_t length) {
    card_romSetCmd(command);
    card_romStartXfer(flags, false);
    if ((uint32_t)buffer & 3)
        card_romCpuWriteUnaligned((uint8_t*)buffer, length);
    else
        card_romCpuWrite((u32*)buffer, length);
}

uint32_t cardExt_ReadData4Byte(uint64_t command, uint32_t flags) {
    card_romSetCmd(command);
    card_romStartXfer(flags | MCCNT1_LEN_4, false);
    card_romWaitDataReady();
    return card_romGetData();
}

void cardExt_SendCommand(uint64_t command, uint32_t flags) {
    card_romSetCmd(command);
    card_romStartXfer(flags | MCCNT1_LEN_0, false);
    card_romWaitBusy();
}

uint8_t cardExt_ReadWriteSpiByte(uint8_t data)
{
    REG_MCD0 = data;
    cardExt_WaitSpiBusy();
    return REG_MCD0;
}
