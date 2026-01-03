#include "common.h"
#include "gameCode.h"
#include "patches/platform/LoaderPlatform.h"
#include "patches/SaveOffsetToSdSectorPatchCode.h"
#include "ReadNandSavePatchCode.h"
#include "WriteNandSavePatchCode.h"
#include "JamWithTheBandNandSavePatch.h"

// This code was based on nds-bootstrap:
// https://github.com/DS-Homebrew/nds-bootstrap/blob/89f27d1392a68436695d0050992ee84258ef41bc/retail/bootloader/source/arm7/patch_arm9.c#L2531

bool JamWithTheBandNandSavePatch::FindPatchTarget(PatchContext& patchContext)
{
    return true;
}

void JamWithTheBandNandSavePatch::ApplyPatch(PatchContext& patchContext)
{
    auto sectorRemapPatchCode = patchContext.GetPatchCodeCollection().AddUniquePatchCode<SaveOffsetToSdSectorPatchCode>
    (
        patchContext.GetPatchHeap(),
        (const save_file_info_t*)((u32)SHARED_SAVE_FILE_INFO - 0x02F00000 + 0x02700000)
    );
    auto loaderPlatform = patchContext.GetLoaderPlatform();
    auto readNandSavePatchCode = patchContext.GetPatchCodeCollection().AddUniquePatchCode<ReadNandSavePatchCode>
    (
        patchContext.GetPatchHeap(),
        sectorRemapPatchCode,
        loaderPlatform->CreateSdReadPatchCode(patchContext.GetPatchCodeCollection(), patchContext.GetPatchHeap())
    );
    auto writeNandSavePatchCode = patchContext.GetPatchCodeCollection().AddUniquePatchCode<WriteNandSavePatchCode>
    (
        patchContext.GetPatchHeap(),
        sectorRemapPatchCode,
        loaderPlatform->CreateSdWritePatchCode(patchContext.GetPatchCodeCollection(), patchContext.GetPatchHeap())
    );

    // u32 nandInit(void* data)
    *(u32*)0x020613CC = 0xE3A00001; // mov r0, #1
    *(u32*)0x020613D0 = 0xE12FFF1E; // bx lr

    // u32 nandResume(void)
    *(u32*)0x02061A4C = 0xE3A00000; // mov r0, #0
    *(u32*)0x02061A50 = 0xE12FFF1E; // bx lr

    // u32 nandError(void)
    *(u32*)0x02061C24 = 0xE3A00000; // mov r0, #0
    *(u32*)0x02061C28 = 0xE12FFF1E; // bx lr

    // u32 nandWrite(const void* memory, u32 flash, u32 size, u32 dmaChannel)
    *(u32*)0x0206176C = 0xE51FF004; // ldr pc,= patch_writeNandSave
    *(u32*)0x02061770 = (u32)writeNandSavePatchCode->GetWriteNandSaveFunction();

    // u32 nandRead(void* memory, u32 flash, u32 size, u32 dmaChannel)
    *(u32*)0x02061AC4 = 0xE51FF004; // ldr pc,= patch_readNandSave
    *(u32*)0x02061AC8 = (u32)readNandSavePatchCode->GetReadNandSaveFunction();
}
