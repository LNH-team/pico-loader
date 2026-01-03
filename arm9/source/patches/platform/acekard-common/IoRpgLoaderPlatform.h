#pragma once
#include "../LoaderPlatform.h"
#include "IoRpgSendSdioCommandPatchCode.h"
#include "IoRpgSdWaitForStatePatchCode.h"

/// @brief Implementation of LoaderPlatform for flashcarts based on the Acekard RPG family
class IoRpgLoaderPlatform : public LoaderPlatform
{
public:
    explicit IoRpgLoaderPlatform(u8 ioRpgCmdSdioByte)
        : _ioRpgCmdSdioByte(ioRpgCmdSdioByte) { }

    LoaderPlatformType GetPlatformType() const override { return LoaderPlatformType::Slot1; }

    bool InitializeSdCard() override;

protected:
    virtual void PatchSdscShift() const {};

private:
    u32 _ioRpgCmdSdioByte;

    /// @brief Sends an SDIO command to the cartridge.
    /// SDIO commands send every bit in a byte in order from MSB to LSB.
    /// @note This function doesn't handle unaligned reads.
    /// @param Card command to be written to REG_MCCMD.
    /// @param Pointer to buffer for the SDIO response.
    /// @param Number of bits in the SDIO response.
    void SdSendSdioCommand(u64 command, u8* buffer, u32 length) const;

    /// @brief Sends an R1 type SDIO command.
    /// This function gets the full R1 response, and truncates it to bits 8 - 39 in a single u32.
    /// QUIRK: RPG doesn't send the MSB. We must assume the MSB is 0, and shift the response >> 1.
    /// @return full R1 response truncated to bits 8 - 39 in a single u32.
    u32 SdSendR1Command(u8 cmd, u32 argument) const;

    /// @brief Sends an R2 type SDIO command.
    /// QUIRK: RPG doesn't send the MSB. We must assume the MSB is 0, and shift the response >> 1.
    /// TODO: actually verify this buffer. We don't actually need it in DLDI, just need to send command and clear the FIFO.
    void SdSendR2Command(u8 cmd, u32 argument) const;

    /// @brief Builds a card command containing the SDIO command, SDIO parameter and the parameter type
    /// @param SDIO command
    /// @param Parameter type seen in IoRpgSdioParamTypes
    /// @param Parameter to SDIO command.
    /// @return A u64 to be written to REG_MCCMD0
    u64 IoRpgCmdSdio(u8 sdio, u8 paramType, u32 parameter) const
    {
        // All AKRPG-based *SD* drivers use SDIO.
        // Format:
        //     0xAABB00CC, 0xDDDDDDDD
        //     AA = IORPG_CMD_SDIO_BYTE // this is different depending on the flashcart
        //     BB = SDIO response type
        //     CC = SDIO command
        //     DDDDDDDD = SDIO parameter
        u32 command0 = (((u32)paramType << 16) | ((u32)sdio));
        return (((u64)_ioRpgCmdSdioByte << 56) | (((u64)command0) << 32) | (u64)parameter);
    }
};
