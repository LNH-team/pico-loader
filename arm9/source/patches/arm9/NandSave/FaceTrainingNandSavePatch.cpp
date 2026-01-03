#include "common.h"
#include "gameCode.h"
#include "patches/platform/LoaderPlatform.h"
#include "patches/SaveOffsetToSdSectorPatchCode.h"
#include "ReadNandSavePatchCode.h"
#include "WriteNandSavePatchCode.h"
#include "FaceTrainingNandSavePatch.h"

// This code was based on nds-bootstrap:
// https://github.com/DS-Homebrew/nds-bootstrap/blob/89f27d1392a68436695d0050992ee84258ef41bc/retail/bootloader/source/arm7/patch_arm9.c#L2531

bool FaceTrainingNandSavePatch::FindPatchTarget(PatchContext& patchContext)
{
    return true;
}

void FaceTrainingNandSavePatch::ApplyPatch(PatchContext& patchContext)
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
    *(u32*)0x020E2AEC = 0xE3A00001; // mov r0, #1
    *(u32*)0x020E2AF0 = 0xE12FFF1E; // bx lr

    // u32 nandResume(void)
    *(u32*)0x020E2EC0 = 0xE3A00000; // mov r0, #0
    *(u32*)0x020E2EC4 = 0xE12FFF1E; // bx lr

    // u32 nandError(void)
    *(u32*)0x020E3150 = 0xE3A00000; // mov r0, #0
    *(u32*)0x020E3154 = 0xE12FFF1E; // bx lr

    // u32 nandWrite(const void* memory, u32 flash, u32 size, u32 dmaChannel)
    *(u32*)0x020E2BF0 = 0xE51FF004; // ldr pc,= patch_writeNandSave
    *(u32*)0x020E2BF4 = (u32)writeNandSavePatchCode->GetWriteNandSaveFunction();

    // u32 nandRead(void* memory, u32 flash, u32 size, u32 dmaChannel)
    *(u32*)0x020E2F3C = 0xE51FF004; // ldr pc,= patch_readNandSave
    *(u32*)0x020E2F40 = (u32)readNandSavePatchCode->GetReadNandSaveFunction();
}
