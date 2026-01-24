#pragma once

#define MODULE_PARAMS_NTR_MAGIC_LE  0x2106C0DE
#define MODULE_PARAMS_NTR_MAGIC_BE  0xDEC00621

#define MODULE_PARAMS_TWL_MAGIC_LE  0x6314C0DE
#define MODULE_PARAMS_TWL_MAGIC_BE  0xDEC01463

struct module_params_ntr_t
{
    u32 autoloadListStart;
    u32 autoloadListEnd;
    u32 autoloadStart;
    u32 bssStart;
    u32 bssEnd;
    u32 compressedEnd;
    u32 sdkVersion;
    u32 magicBigEndian;         //0xDEC00621
    u32 magicLittleEndian;      //0x2106C0DE
};

// for the twl binaries of a hybrid or limited rom
struct module_params_twl_t
{
    u32 autoloadListStart;
    u32 autoloadListEnd;
    u32 autoloadStart;
    u32 compressedEnd;
    u32 magicBigEndian;         //0xDEC01463
    u32 magicLittleEndian;      //0x6314C0DE
};

// only on sdk 5
struct build_params_t
{
    u32 reserved0[4];
    u8 buildCode;
    u8 sdkTarget;
    u8 reserved1[2];
    u8 version;
    u8 reserved2[3];
    u32 magicBigEndian;         //0xDEC08133
    u32 magicLittleEndian;      //0x3381C0DE
};

struct autoload_list_entry_t
{
    u32 targetAddress;
    u32 size;
    u32 bssSize;
};

struct autoload_list_entry_sdk5_t
{
    u32 targetAddress;
    u32 size;
    u32 staticInitStart;
    u32 bssSize;
};
