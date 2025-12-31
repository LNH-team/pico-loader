#include "common.h"
#include <libtwl/sys/twlScfg.h>
#include <libtwl/sys/twlFuse.h>
#include <libtwl/dma/dmaTwl.h>
#include "TwlAes.h"

#define KEY_SLOT_MODULE         0
#define KEY_SLOT_NAND           3

#define MODULE_KEYX_NINT        0x746E694E  // Nint
#define MODULE_KEYX_ENDO        0x6F646E65  // endo

#define NAND_KEYX_DSI_XOR_LO    0x24EE6906
#define NAND_KEYX_DSI_XOR_HI    0xE65B601D

#define NAND_KEYX_3DS_NINT      0x544E494E  // NINT
#define NAND_KEYX_3DS_ENDO      0x4F444E45  // ENDO

#define NAND_KEYY_WORD_3        0xE1A00005  // mov r0, r5

#define DMA_CHANNEL_AES_OUT     0
#define DMA_CHANNEL_AES_IN      1

void TwlAes::SetupAes(const nds_header_twl_t* romHeader) const
{
    // ensure AES is enabled
    REG_SCFG_EXT |= SCFG_EXT_AES;
    REG_SCFG_CLK |= SCFG_CLK_AES;

    REG_AES_CNT = 0;

    aes_reset();
    aes_reset();
    aes_waitKeyBusy();

    SetupModuleKeyXY(romHeader);
    SetupNandKeyX();
    aes_waitKeyBusy();
    (&REG_AES_SEED0)[KEY_SLOT_NAND * 3].words[3] = NAND_KEYY_WORD_3;
}

void TwlAes::SetupKeySlot(u32 keySlot, const aes_u128_t* key) const
{
    REG_AES_CNT = 0;

    aes_reset();
    aes_reset();
    aes_waitKeyBusy();

    aes_setKey(keySlot, key);

    aes_setKeySlot(keySlot);
    aes_waitKeyBusy();
}

void TwlAes::DecryptModuleAes(void* data, u32 length, const aes_u128_t* iv) const
{
    REG_AES_CNT = 0;

    aes_reset();
    aes_reset();
    aes_waitKeyBusy();

    aes_setKeySlot(0);
    aes_waitKeyBusy();

    u32 offset = 0;
    while (length > 0)
    {
        REG_AES_CNT = 0;
        aes_reset();
        u32 blockLength = std::min<u32>(length, 0xFFFF0u);

        auto ctr = *iv;
        ((u64*)&ctr)[1] += __builtin_add_overflow(((u64*)&ctr)[0], offset >> 4, &((u64*)&ctr)[0]);
        aes_setInitializationVector(&ctr);
        aes_setPayloadBlockCount(blockLength >> 4);

        LOG_DEBUG("%p\n", (u8*)data + offset);

        dma_twl_config_t inputDmaConfig
        {
            .src = (u8*)data + offset,
            .dst = (void*)&REG_AES_IFIFO,
            .totalWordCount = blockLength >> 2,
            .wordCount = 4,
            .blockInterval = NDMABCNT_INTERVAL(8),
            .fillData = 0,
            .control = NDMACNT_DST_MODE_FIXED | NDMACNT_SRC_MODE_INCREMENT |
                NDMACNT_PHYSICAL_COUNT_4 | NDMACNT_MODE_AES_IN | NDMACNT_ENABLE
        };
        dma_twlSetParams(DMA_CHANNEL_AES_IN, &inputDmaConfig);

        dma_twl_config_t outputDmaConfig
        {
            .src = (const void*)&REG_AES_OFIFO,
            .dst = (u8*)data + offset,
            .totalWordCount = blockLength >> 2,
            .wordCount = 4,
            .blockInterval = NDMABCNT_INTERVAL(8),
            .fillData = 0,
            .control = NDMACNT_SRC_MODE_FIXED | NDMACNT_DST_MODE_INCREMENT |
                NDMACNT_PHYSICAL_COUNT_4 | NDMACNT_MODE_AES_OUT | NDMACNT_ENABLE
        };
        dma_twlSetParams(DMA_CHANNEL_AES_OUT, &outputDmaConfig);

        aes_start(
            AES_CNT_INPUT_FIFO_DMA_SIZE_4_BYTES |
            AES_CNT_OUTPUT_FIFO_DMA_SIZE_4_BYTES |
            AES_CNT_MODE_CTR);
        dma_twlWait(DMA_CHANNEL_AES_OUT);
        aes_waitBusy();

        offset += blockLength;
        length -= blockLength;
    }
}

void TwlAes::SetupModuleKeyXY(const nds_header_twl_t* romHeader) const
{
    if ((romHeader->twlFlags & (1 << 2)) || (romHeader->twlFlags2 & (1 << 7)))
    {
        // debug
        aes_setKey(KEY_SLOT_MODULE, (const aes_u128_t*)romHeader);
    }
    else
    {
        // retail
        aes_u128_t keyX { .words =
        {
            MODULE_KEYX_NINT,
            MODULE_KEYX_ENDO,
            romHeader->gameCode,
            __builtin_bswap32(romHeader->gameCode)
        }};
        aes_setKeyXY(KEY_SLOT_MODULE, &keyX, (const aes_u128_t*)romHeader->arm9iSha1Hmac);
    }
}

void TwlAes::SetupNandKeyX() const
{
    if ((REG_SCFG_A7ROM & SCFG_A7ROM_DISABLE_FUSE) || (REG_FUSE_VERIFY & FUSE_VERIFY_ERROR))
    {
        // No access to the console id register. We'll assume nand key x is already setup.
        return;
    }

    u32 consoleIdLo = REG_FUSE_ID0;
    u32 consoleIdHi = REG_FUSE_ID1;

    aes_u128_t keyX;
    keyX.words[0] = consoleIdLo;
    if (consoleIdLo & 0x80000000)
    {
        // 3DS
        keyX.words[1] = NAND_KEYX_3DS_NINT;
        keyX.words[2] = NAND_KEYX_3DS_ENDO;
    }
    else
    {
        // DSi
        keyX.words[1] = consoleIdLo ^ NAND_KEYX_DSI_XOR_LO;
        keyX.words[2] = consoleIdHi ^ NAND_KEYX_DSI_XOR_HI;
    }
    keyX.words[3] = consoleIdHi;
    aes_setKeyX(KEY_SLOT_NAND, &keyX);
}
