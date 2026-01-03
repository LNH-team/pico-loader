#include "common.h"
#include "gameCode.h"
#include "patches/platform/LoaderPlatform.h"
#include "patches/SaveOffsetToSdSectorPatchCode.h"
#include "ReadNandSavePatchCode.h"
#include "WriteNandSavePatchCode.h"
#include "NintendoDSGuideNandSavePatch.h"

// This code was based on nds-bootstrap:
// https://github.com/DS-Homebrew/nds-bootstrap/blob/89f27d1392a68436695d0050992ee84258ef41bc/retail/bootloader/source/arm7/patch_arm9.c#L2531

bool NintendoDSGuideNandSavePatch::FindPatchTarget(PatchContext& patchContext)
{
    return true;
}

void NintendoDSGuideNandSavePatch::ApplyPatch(PatchContext& patchContext)
{
    auto sectorRemapPatchCode = patchContext.GetPatchCodeCollection().AddUniquePatchCode<SaveOffsetToSdSectorPatchCode>
    (
        patchContext.GetPatchHeap(),
        SHARED_SAVE_FILE_INFO
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
    *(u32*)0x02009298 = 0xE3A00001; // mov r0, #1
    *(u32*)0x0200929C = 0xE12FFF1E; // bx lr

    // u32 nandResume(void)
    *(u32*)0x020098C8 = 0xE3A00000; // mov r0, #0
    *(u32*)0x020098CC = 0xE12FFF1E; // bx lr

    // u32 nandState(void)
    *(u32*)0x02009AA8 = 0xE1A00000; //nop
    *(u32*)0x02009AB0 = 0x06600000;

    // u32 nandError(void)
    *(u32*)0x02009AB4 = 0xE3A00000; // mov r0, #0
    *(u32*)0x02009AB8 = 0xE12FFF1E; // bx lr

    // u32 nandWrite(const void* memory, u32 flash, u32 size, u32 dmaChannel)
    *(u32*)0x0200961C = 0xE51FF004; // ldr pc,= patch_writeNandSave
    *(u32*)0x02009620 = (u32)writeNandSavePatchCode->GetWriteNandSaveFunction();

    // u32 nandRead(void* memory, u32 flash, u32 size, u32 dmaChannel)
    *(u32*)0x02009940 = 0xE51FF004; // ldr pc,= patch_readNandSave
    *(u32*)0x02009944 = (u32)readNandSavePatchCode->GetReadNandSaveFunction();
}
