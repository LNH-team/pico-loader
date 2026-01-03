#include "common.h"
#include "gameCode.h"
#include "patches/platform/LoaderPlatform.h"
#include "patches/SaveOffsetToSdSectorPatchCode.h"
#include "ReadNandSavePatchCode.h"
#include "WriteNandSavePatchCode.h"
#include "WarioWareDiyNandSavePatch.h"

// This code was based on nds-bootstrap:
// https://github.com/DS-Homebrew/nds-bootstrap/blob/89f27d1392a68436695d0050992ee84258ef41bc/retail/bootloader/source/arm7/patch_arm9.c#L2531

bool WarioWareDiyNandSavePatch::FindPatchTarget(PatchContext& patchContext)
{
    _sdPatchEntry = nullptr;
    switch (patchContext.GetGameCode())
    {
        case GAMECODE("UORE"):
        {
            _sdPatchEntry = (u8*)0x02002C04;
            break;
        }
        case GAMECODE("UORP"):
        {
            _sdPatchEntry = (u8*)0x02002CA4;
            break;
        }
        case GAMECODE("UORJ"):
        {
            _sdPatchEntry = (u8*)0x02002BE4;
            break;
        }
    }

    return _sdPatchEntry != nullptr;
}

void WarioWareDiyNandSavePatch::ApplyPatch(PatchContext& patchContext)
{
    if (_sdPatchEntry == nullptr)
    {
        return;
    }

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
    *(u32*)(_sdPatchEntry + 0x50C) = 0xE3A00001; // mov r0, #1
    *(u32*)(_sdPatchEntry + 0x510) = 0xE12FFF1E; // bx lr

    // u32 nandWait(void)
    *(u32*)(_sdPatchEntry + 0xC9C) = 0xE12FFF1E; // bx lr

    // u32 nandState(void)
    *(u32*)(_sdPatchEntry + 0xEB0) = 0xE3A00003; // mov r0, #3
    *(u32*)(_sdPatchEntry + 0xEB4) = 0xE12FFF1E; // bx lr

    // u32 nandError(void)
    *(u32*)(_sdPatchEntry + 0xEC8) = 0xE3A00000; // mov r0, #0
    *(u32*)(_sdPatchEntry + 0xECC) = 0xE12FFF1E; // bx lr

    // u32 nandWrite(void* memory, u32 flash, u32 size, u32 dmaChannel)
    *(u32*)(_sdPatchEntry + 0x958) = 0xE51FF004; // ldr pc,= patch_writeNandSave
    *(u32*)(_sdPatchEntry + 0x95C) = (u32)writeNandSavePatchCode->GetWriteNandSaveFunction();

    // u32 nandRead(void* memory, u32 flash, u32 size, u32 dmaChannel)
    *(u32*)(_sdPatchEntry + 0xD24) = 0xE51FF004; // ldr pc,= patch_readNandSave
    *(u32*)(_sdPatchEntry + 0xD28) = (u32)readNandSavePatchCode->GetReadNandSaveFunction();
}
