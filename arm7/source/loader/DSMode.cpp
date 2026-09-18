#include "common.h"
#include <libtwl/i2c/i2cMcu.h>
#include <libtwl/sound/twlI2s.h>
#include <libtwl/sound/sound.h>
#include <libtwl/spi/spiCodec.h>
#include <libtwl/spi/spiPmic.h>
#include <libtwl/sys/twlScfg.h>
#include "ipc.h"
#include "gameCode.h"
#include "DSMode.h"

void DSMode::SwitchToDSMode(u32 gameCode) const
{
    SwitchToDSTouchAndSoundMode(gameCode);
    mcu_writeReg(MCU_REG_MODE, 0);
    *(vu16*)0x04004C04 |= (1 << 8); // ntr wifi
    REG_SCFG_A9ROM = SCFG_A9ROM_DISABLE_SECURE | SCFG_A9ROM_NITRO;
    REG_SCFG_A7ROM = SCFG_A7ROM_DISABLE_SECURE | SCFG_A7ROM_NITRO | SCFG_A7ROM_DISABLE_FUSE;
    REG_SCFG_EXT = 0x92A03000u;
    sendToArm9(IPC_COMMAND_ARM9_SWITCH_TO_DS_MODE);
    receiveFromArm9();

    LOG_DEBUG("Switched to ds mode\n");
}

void DSMode::SwitchToDSTouchAndSoundMode(u32 gameCode) const
{
    REG_I2SCNT = I2SCNT_MIX_RATIO_DSP_0_NITRO_8 | I2SCNT_FREQUENCY_32728_HZ;
    codec_setPage(CODEC_PAGE_0);
    {
        codec_writeRegister(CODEC_REG_PAGE0_DAC_NDAC_VAL, 0x87);
        codec_writeRegister(CODEC_REG_PAGE0_ADC_NADC_VAL, 0x87);
        codec_writeRegister(CODEC_REG_PAGE0_PLL_J, 21);
    }
    REG_I2SCNT |= I2SCNT_ENABLE;
    SwitchCodecToDSMode(gameCode);
    REG_SOUNDCNT = SOUNDCNT_MASTER_ENABLE | SOUNDCNT_MASTER_VOLUME(0x7F);

    LOG_DEBUG("Switched to ds touch and sound mode\n");
}

void DSMode::SwitchCodecToDSMode(u32 gameCode) const
{
    // 0xAC: special setting (when found special gamecode)
    // 0xA7: normal setting (for any other gamecodes)
    u8 volLevel = ShouldUseVolumeFix(gameCode) ? 0xAC : 0xA7;

    // Touchscreen
    codec_setPage(CODEC_PAGE_0);
    {
        codec_writeRegister(CODEC_REG_PAGE0_GPI3_PIN_CONTROL, 0x00);
        codec_readRegister(CODEC_REG_PAGE0_ADC_DIGITAL_MIC);
    }
    codec_setPage(CODEC_PAGE_3);
    {
        codec_readRegister(CODEC_REG_PAGE3_SAR_ADC_CONTROL_1);
    }
    codec_setPage(CODEC_PAGE_0);
    {
        codec_readRegister(CODEC_REG_PAGE0_DAC_DATA_PATH_SETUP);
    }
    codec_setPage(CODEC_PAGE_1);
    {
        codec_readRegister(CODEC_REG_PAGE1_HPL_DRIVER);
        codec_readRegister(CODEC_REG_PAGE1_SPL_DRIVER);
        codec_readRegister(CODEC_REG_PAGE1_MICBIAS);
    }
    codec_setPage(CODEC_PAGE_0);
    {
        codec_writeRegister(CODEC_REG_PAGE0_ADC_DIGITAL_VOLUME_CONTROL_FINE_ADJUST, 0x80);
        codec_writeRegister(CODEC_REG_PAGE0_DAC_VOLUME_CONTROL, 0x0C);
    }
    codec_setPage(CODEC_PAGE_1);
    {
        codec_writeRegister(CODEC_REG_PAGE1_LEFT_ANALOG_VOL_TO_HPL, 0xFF);
        codec_writeRegister(CODEC_REG_PAGE1_RIGHT_ANALOG_VOL_TO_HPR, 0xFF);
        codec_writeRegister(CODEC_REG_PAGE1_LEFT_ANALOG_VOL_TO_SPL, 0x7F);
        codec_writeRegister(CODEC_REG_PAGE1_RIGHT_ANALOG_VOL_TO_SPR, 0x7F);
        codec_writeRegister(CODEC_REG_PAGE1_HPL_DRIVER, 0x4A);
        codec_writeRegister(CODEC_REG_PAGE1_HPR_DRIVER, 0x4A);
        codec_writeRegister(CODEC_REG_PAGE1_SPL_DRIVER, 0x10);
        codec_writeRegister(CODEC_REG_PAGE1_SPR_DRIVER, 0x10);
    }
    codec_setPage(CODEC_PAGE_0);
    {
        codec_writeRegister(CODEC_REG_PAGE0_ADC_DIGITAL_MIC, 0x00);
    }
    codec_setPage(CODEC_PAGE_3);
    {
        codec_readRegister(CODEC_REG_PAGE3_SAR_ADC_CONTROL_1);
        codec_writeRegister(CODEC_REG_PAGE3_SAR_ADC_CONTROL_1, 0x98);
    }
    codec_setPage(CODEC_PAGE_1);
    {
        codec_writeRegister(CODEC_REG_PAGE1_DAC_L_DAC_R_OUTPUT_MIXER_ROUTING, 0x00);
        codec_writeRegister(CODEC_REG_PAGE1_HEADPHONE_DRIVERS, 0x14);
        codec_writeRegister(CODEC_REG_PAGE1_CLASS_D_SPEAKER_AMPLIFIER, 0x14);
    }
    codec_setPage(CODEC_PAGE_0);
    {
        codec_writeRegister(CODEC_REG_PAGE0_DAC_DATA_PATH_SETUP, 0x00);
        codec_readRegister(CODEC_REG_PAGE0_DAC_NDAC_VAL);
        codec_writeRegister(CODEC_REG_PAGE0_PLL_P_R, 0x00);
        codec_writeRegister(CODEC_REG_PAGE0_DAC_NDAC_VAL, 0x01);
        codec_writeRegister(CODEC_REG_PAGE0_DAC_MDAC_VAL, 0x02);
        codec_writeRegister(CODEC_REG_PAGE0_ADC_NADC_VAL, 0x01);
        codec_writeRegister(CODEC_REG_PAGE0_ADC_MADC_VAL, 0x02);
    }
    codec_setPage(CODEC_PAGE_1);
    {
        codec_writeRegister(CODEC_REG_PAGE1_MICBIAS, 0x00);
    }
    codec_setPage(CODEC_PAGE_0);
    {
        codec_writeRegister(CODEC_REG_PAGE0_GPI3_PIN_CONTROL, 0x60);
        codec_writeRegister(CODEC_REG_PAGE0_RESET, 0x01);
        codec_writeRegister(CODEC_REG_PAGE0_GPI1_GPI2_PIN_CONTROL, 0x66);
    }
    codec_setPage(CODEC_PAGE_1);
    {
        codec_readRegister(CODEC_REG_PAGE1_CLASS_D_SPEAKER_AMPLIFIER);
        codec_writeRegister(CODEC_REG_PAGE1_CLASS_D_SPEAKER_AMPLIFIER, 0x10);
    }
    codec_setPage(CODEC_PAGE_0);
    {
        codec_writeRegister(CODEC_REG_PAGE0_CLOCK_GEN_MUXING, 0x00);
        codec_writeRegister(CODEC_REG_PAGE0_ADC_NADC_VAL, 0x81);
        codec_writeRegister(CODEC_REG_PAGE0_ADC_MADC_VAL, 0x82);
        codec_writeRegister(CODEC_REG_PAGE0_ADC_DIGITAL_MIC, 0x82);
        codec_writeRegister(CODEC_REG_PAGE0_ADC_DIGITAL_MIC, 0x00);
        codec_writeRegister(CODEC_REG_PAGE0_CLOCK_GEN_MUXING, 0x03);
        codec_writeRegister(CODEC_REG_PAGE0_PLL_P_R, 0xA1);
        codec_writeRegister(CODEC_REG_PAGE0_PLL_J, 0x15);
        codec_writeRegister(CODEC_REG_PAGE0_DAC_NDAC_VAL, 0x87);
        codec_writeRegister(CODEC_REG_PAGE0_DAC_MDAC_VAL, 0x83);
        codec_writeRegister(CODEC_REG_PAGE0_ADC_NADC_VAL, 0x87);
        codec_writeRegister(CODEC_REG_PAGE0_ADC_MADC_VAL, 0x83);
    }
    codec_setPage(CODEC_PAGE_3);
    {
        codec_readRegister(CODEC_REG_PAGE3_SCAN_MODE_TIMER_CLOCK);
        codec_writeRegister(CODEC_REG_PAGE3_SCAN_MODE_TIMER_CLOCK, 0x08);
    }
    codec_setPage(CODEC_PAGE_4);
    {
        codec_writeRegister(0x08, 0x7F);
        codec_writeRegister(0x09, 0xE1);
        codec_writeRegister(0x0A, 0x80);
        codec_writeRegister(0x0B, 0x1F);
        codec_writeRegister(0x0C, 0x7F);
        codec_writeRegister(0x0D, 0xC1);
    }
    codec_setPage(CODEC_PAGE_0);
    {
        codec_writeRegister(CODEC_REG_PAGE0_DAC_LEFT_VOLUME_CONTROL, 0x08);
        codec_writeRegister(CODEC_REG_PAGE0_DAC_RIGHT_VOLUME_CONTROL, 0x08);
        codec_writeRegister(CODEC_REG_PAGE0_GPI3_PIN_CONTROL, 0x00);
    }
    codec_setPage(CODEC_PAGE_4);
    {
        codec_writeRegister(0x08, 0x7F);
        codec_writeRegister(0x09, 0xE1);
        codec_writeRegister(0x0A, 0x80);
        codec_writeRegister(0x0B, 0x1F);
        codec_writeRegister(0x0C, 0x7F);
        codec_writeRegister(0x0D, 0xC1);
    }
    codec_setPage(CODEC_PAGE_1);
    {
        codec_writeRegister(CODEC_REG_PAGE1_MIC_PGA, 0x2B);
        codec_writeRegister(CODEC_REG_PAGE1_DELTA_SIGMA_MONO_ADC_CHANNEL_FINE_GAIN_INPUT_SELECTION_FOR_P_TERMINAL, 0x40);
        codec_writeRegister(CODEC_REG_PAGE1_ADC_INPUT_SELECTION_FOR_M_TERMINAL, 0x40);
        codec_writeRegister(CODEC_REG_PAGE1_INPUT_CM_SETTINGS, 0x60);
    }
    codec_setPage(CODEC_PAGE_0);
    {
        codec_readRegister(CODEC_REG_PAGE0_VOL_MICDET_PIN_SAR_ADC_VOLUME_CONTROL);
        codec_writeRegister(CODEC_REG_PAGE0_VOL_MICDET_PIN_SAR_ADC_VOLUME_CONTROL, 0x02);
        codec_readRegister(CODEC_REG_PAGE0_VOL_MICDET_PIN_SAR_ADC_VOLUME_CONTROL);
        codec_writeRegister(CODEC_REG_PAGE0_VOL_MICDET_PIN_SAR_ADC_VOLUME_CONTROL, 0x10);
        codec_readRegister(CODEC_REG_PAGE0_VOL_MICDET_PIN_SAR_ADC_VOLUME_CONTROL);
        codec_writeRegister(CODEC_REG_PAGE0_VOL_MICDET_PIN_SAR_ADC_VOLUME_CONTROL, 0x40);
    }
    codec_setPage(CODEC_PAGE_1);
    {
        codec_writeRegister(CODEC_REG_PAGE1_HP_OUTPUT_DRIVERS_POP_REMOVAL_SETTINGS, 0x20);
        codec_writeRegister(CODEC_REG_PAGE1_OUTPUT_DRIVER_PGA_RAMP_DOWN_PERIOD_CONTROL, 0xF0);
    }
    codec_setPage(CODEC_PAGE_0);
    {
        codec_readRegister(CODEC_REG_PAGE0_ADC_DIGITAL_MIC);
        codec_readRegister(CODEC_REG_PAGE0_DAC_DATA_PATH_SETUP);
        codec_writeRegister(CODEC_REG_PAGE0_DAC_DATA_PATH_SETUP, 0xD4);
    }
    codec_setPage(CODEC_PAGE_1);
    {
        codec_writeRegister(CODEC_REG_PAGE1_DAC_L_DAC_R_OUTPUT_MIXER_ROUTING, 0x44);
        codec_writeRegister(CODEC_REG_PAGE1_HEADPHONE_DRIVERS, 0xD4);
        codec_writeRegister(CODEC_REG_PAGE1_HPL_DRIVER, 0x4E);
        codec_writeRegister(CODEC_REG_PAGE1_HPR_DRIVER, 0x4E);
        codec_writeRegister(CODEC_REG_PAGE1_LEFT_ANALOG_VOL_TO_HPL, 0x9E);
        codec_writeRegister(CODEC_REG_PAGE1_RIGHT_ANALOG_VOL_TO_HPR, 0x9E);
        codec_writeRegister(CODEC_REG_PAGE1_CLASS_D_SPEAKER_AMPLIFIER, 0xD4);
        codec_writeRegister(CODEC_REG_PAGE1_SPL_DRIVER, 0x14);
        codec_writeRegister(CODEC_REG_PAGE1_SPR_DRIVER, 0x14);
        codec_writeRegister(CODEC_REG_PAGE1_LEFT_ANALOG_VOL_TO_SPL, 0xA7);
        codec_writeRegister(CODEC_REG_PAGE1_RIGHT_ANALOG_VOL_TO_SPR, 0xA7);
    }
    codec_setPage(CODEC_PAGE_0);
    {
        codec_writeRegister(CODEC_REG_PAGE0_DAC_VOLUME_CONTROL, 0x00);
        codec_writeRegister(CODEC_REG_PAGE0_GPI3_PIN_CONTROL, 0x60);
    }
    codec_setPage(CODEC_PAGE_1);
    {
        codec_writeRegister(CODEC_REG_PAGE1_LEFT_ANALOG_VOL_TO_SPL, volLevel);
        codec_writeRegister(CODEC_REG_PAGE1_RIGHT_ANALOG_VOL_TO_SPR, volLevel);
        codec_writeRegister(CODEC_REG_PAGE1_MICBIAS, 0x03);
    }
    codec_setPage(CODEC_PAGE_3);
    {
        codec_writeRegister(CODEC_REG_PAGE3_SAR_ADC_CONTROL_2, 0x00);
    }
    codec_setPage(CODEC_PAGE_1);
    {
        codec_writeRegister(CODEC_REG_PAGE1_HP_OUTPUT_DRIVERS_POP_REMOVAL_SETTINGS, 0x20);
        codec_writeRegister(CODEC_REG_PAGE1_OUTPUT_DRIVER_PGA_RAMP_DOWN_PERIOD_CONTROL, 0xF0);
        codec_readRegister(CODEC_REG_PAGE1_OUTPUT_DRIVER_PGA_RAMP_DOWN_PERIOD_CONTROL);
        codec_writeRegister(CODEC_REG_PAGE1_OUTPUT_DRIVER_PGA_RAMP_DOWN_PERIOD_CONTROL, 0x00);
    }
    codec_setPage(CODEC_PAGE_0);
    {
        codec_writeRegister(CODEC_REG_PAGE0_ADC_DIGITAL_VOLUME_CONTROL_FINE_ADJUST, 0x80);
        codec_writeRegister(CODEC_REG_PAGE0_ADC_DIGITAL_MIC, 0x00);

        // Set remaining values
        codec_writeRegister(0x03, 0x44);
        codec_writeRegister(CODEC_REG_PAGE0_DAC_DOSR_VAL_MSB, 0x00);
        codec_writeRegister(CODEC_REG_PAGE0_DAC_DOSR_VAL_LSB, 0x80);
        codec_writeRegister(CODEC_REG_PAGE0_DAC_IDAC_VAL, 0x80);
        codec_writeRegister(CODEC_REG_PAGE0_DAC_MINIDSP_INTERPOLATION, 0x08);
        codec_writeRegister(CODEC_REG_PAGE0_ADC_AOSR_VAL, 0x80);
        codec_writeRegister(CODEC_REG_PAGE0_ADC_IDAC_VAL, 0x80);
        codec_writeRegister(CODEC_REG_PAGE0_ADC_MINIDSP_DECIMATION, 0x04);
        codec_writeRegister(CODEC_REG_PAGE0_CLKOUT_M_VAL, 0x01);
        codec_writeRegister(CODEC_REG_PAGE0_BCLK_N_VAL, 0x01);
        codec_writeRegister(CODEC_REG_PAGE0_ADC_FLAG_REGISTER, 0x80);
        codec_writeRegister(CODEC_REG_PAGE0_GPIO1_IN_OUT_PIN_CONTROL, 0x34);
        codec_writeRegister(CODEC_REG_PAGE0_GPIO2_IN_OUT_PIN_CONTROL, 0x32);
        codec_writeRegister(CODEC_REG_PAGE0_SDOUT_OUT_PIN_CONTROL, 0x12);
        codec_writeRegister(CODEC_REG_PAGE0_SDIN_IN_PIN_CONTROL, 0x03);
        codec_writeRegister(CODEC_REG_PAGE0_MISO_OUT_PIN_CONTROL, 0x02);
        codec_writeRegister(CODEC_REG_PAGE0_SCLK_IN_PIN_CONTROL, 0x03);
        codec_writeRegister(CODEC_REG_PAGE0_DAC_INSTRUCTION_SET, 0x19);
        codec_writeRegister(CODEC_REG_PAGE0_ADC_INSTRUCTION_SET, 0x05);
        codec_writeRegister(CODEC_REG_PAGE0_DRC_CONTROL_1, 0x0F);
        codec_writeRegister(CODEC_REG_PAGE0_DRC_CONTROL_2, 0x38);
        codec_writeRegister(CODEC_REG_PAGE0_BEEP_LENGTH_MSB, 0x00);
        codec_writeRegister(CODEC_REG_PAGE0_BEEP_LENGTH_MID, 0x00);
        codec_writeRegister(CODEC_REG_PAGE0_BEEP_LENGTH_LSB, 0xEE);
        codec_writeRegister(CODEC_REG_PAGE0_BEEP_SIN_MSB, 0x10);
        codec_writeRegister(CODEC_REG_PAGE0_BEEP_SIN_LSB, 0xD8);
        codec_writeRegister(CODEC_REG_PAGE0_BEEP_COS_MSB, 0x7E);
        codec_writeRegister(CODEC_REG_PAGE0_BEEP_COS_LSB, 0xE3);
        codec_writeRegister(CODEC_REG_PAGE0_AGC_MAXIMUM_GAIN, 0x7F);
        codec_writeRegister(CODEC_REG_PAGE0_VOL_MICDET_PIN_SAR_ADC_VOLUME_CONTROL, 0xD2);
        codec_writeRegister(CODEC_REG_PAGE0_VOL_MICDET_PIN_GAIN, 0x2C);
    }
    codec_setPage(CODEC_PAGE_1);
    {
        codec_writeRegister(CODEC_REG_PAGE1_OUTPUT_DRIVER_PGA_RAMP_DOWN_PERIOD_CONTROL, 0x70);
        codec_writeRegister(CODEC_REG_PAGE1_HP_DRIVER_CONTROL, 0x20);
    }

    // Finish up!
    codec_setPage(CODEC_PAGE_3);
    {
        codec_readRegister(CODEC_REG_PAGE3_SAR_ADC_CONTROL_1);
        codec_writeRegister(CODEC_REG_PAGE3_SAR_ADC_CONTROL_1, 0x98);
    }
    codec_setPage(CODEC_PAGE_255);
    {
        codec_writeRegister(CODEC_REG_PAGE255_BACKWARDS_COMPATIBILITY_MODE, CODEC_PAGE255_BACKWARDS_COMPATIBILITY_MODE_ON);
    }

    pmic_setAmplifierEnable(true);
}

bool DSMode::ShouldUseVolumeFix(u32 gameCode) const
{
    switch (gameCode & 0xFFFFFF)
    {
        case GAMECODE_NO_REGION("A3T"):
        case GAMECODE_NO_REGION("A4U"):
        case GAMECODE_NO_REGION("A5H"):
        case GAMECODE_NO_REGION("A5I"):
        case GAMECODE_NO_REGION("A8N"):
        case GAMECODE_NO_REGION("ABJ"):
        case GAMECODE_NO_REGION("ABN"):
        case GAMECODE_NO_REGION("ABX"):
        case GAMECODE_NO_REGION("ACC"):
        case GAMECODE_NO_REGION("ACL"):
        case GAMECODE_NO_REGION("ACZ"):
        case GAMECODE_NO_REGION("ADA"):
        case GAMECODE_NO_REGION("AHD"):
        case GAMECODE_NO_REGION("AJU"):
        case GAMECODE_NO_REGION("AKA"):
        case GAMECODE_NO_REGION("AKE"):
        case GAMECODE_NO_REGION("ALH"):
        case GAMECODE_NO_REGION("AMH"):
        case GAMECODE_NO_REGION("AN9"):
        case GAMECODE_NO_REGION("ANR"):
        case GAMECODE_NO_REGION("APA"):
        case GAMECODE_NO_REGION("APY"):
        case GAMECODE_NO_REGION("ART"):
        case GAMECODE_NO_REGION("AV2"):
        case GAMECODE_NO_REGION("AV3"):
        case GAMECODE_NO_REGION("AV4"):
        case GAMECODE_NO_REGION("AV5"):
        case GAMECODE_NO_REGION("AV6"):
        case GAMECODE_NO_REGION("AVI"):
        case GAMECODE_NO_REGION("AVT"):
        case GAMECODE_NO_REGION("AWH"):
        case GAMECODE_NO_REGION("AWY"):
        case GAMECODE_NO_REGION("AXB"):
        case GAMECODE_NO_REGION("AXJ"):
        case GAMECODE_NO_REGION("AY7"):
        case GAMECODE_NO_REGION("AYK"):
        case GAMECODE_NO_REGION("AZW"):
        case GAMECODE_NO_REGION("CPU"):
        case GAMECODE_NO_REGION("YB2"):
        case GAMECODE_NO_REGION("YB3"):
        case GAMECODE_NO_REGION("YBO"):
        case GAMECODE_NO_REGION("YCH"):
        case GAMECODE_NO_REGION("YCQ"):
        case GAMECODE_NO_REGION("YFE"):
        case GAMECODE_NO_REGION("YFS"):
        case GAMECODE_NO_REGION("YG8"):
        case GAMECODE_NO_REGION("YGD"):
        case GAMECODE_NO_REGION("YKR"):
        case GAMECODE_NO_REGION("YNZ"):
        case GAMECODE_NO_REGION("YO9"):
        case GAMECODE_NO_REGION("YON"):
        case GAMECODE_NO_REGION("YRM"):
        case GAMECODE_NO_REGION("YT3"):
        case GAMECODE_NO_REGION("YW2"):
        case GAMECODE_NO_REGION("YYK"):
        {
            return true;
        }
        default:
        {
            return false;
        }
    }
}
