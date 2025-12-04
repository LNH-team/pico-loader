#include "common.h"
#include "patches/platform/LoaderPlatform.h"
#include "patches/platform/dspico/DSPicoLoaderPlatform.h"
#include "patches/platform/isnitro/IsNitroLoaderPlatform.h"
#include "patches/platform/m3ds/M3DSLoaderPlatform.h"
#include "patches/platform/r4/R4LoaderPlatform.h"
#include "patches/platform/ace3ds/Ace3DSLoaderPlatform.h"
#include "patches/platform/g003/G003LoaderPlatform.h"
#include "patches/platform/melonds/MelonDSLoaderPlatform.h"
#include "patches/platform/dstt/DSTTLoaderPlatform.h"
#include "patches/platform/ak2/AK2LoaderPlatform.h"
#include "patches/platform/akrpg/AKRPGLoaderPlatform.h"
#include "patches/platform/r4idsn/R4iDSNLoaderPlatform.h"
#include "patches/platform/supercard/SuperCardLoaderPlatform.h"
#include "patches/platform/ezp/EZPLoaderPlatform.h"
#include "LoaderPlatformFactory.h"

LoaderPlatform* LoaderPlatformFactory::CreateLoaderPlatform() const
{
#if defined(PICO_LOADER_TARGET_ISNITRO)
    return new IsNitroLoaderPlatform();
#elif defined(PICO_LOADER_TARGET_DSPICO)
    return new DSPicoLoaderPlatform();
#elif defined(PICO_LOADER_TARGET_M3DS)
    return new M3DSLoaderPlatform();
#elif defined(PICO_LOADER_TARGET_R4)
    return new R4LoaderPlatform();
#elif defined(PICO_LOADER_TARGET_ACE3DS)
    return new Ace3DSLoaderPlatform();
#elif defined(PICO_LOADER_TARGET_G003)
    return new G003LoaderPlatform();
#elif defined(PICO_LOADER_TARGET_MELONDS)
    return new MelonDSLoaderPlatform();
#elif defined(PICO_LOADER_TARGET_DSTT)
    return new DSTTLoaderPlatform();
#elif defined(PICO_LOADER_TARGET_AK2)
    return new AK2LoaderPlatform();
#elif defined(PICO_LOADER_TARGET_AKRPG)
    return new AKRPGLoaderPlatform();
#elif defined(PICO_LOADER_TARGET_R4iDSN)
    return new R4iDSNLoaderPlatform();
#elif defined(PICO_LOADER_TARGET_SUPERCARD)
    return new SuperCardLoaderPlatform();
#elif defined(PICO_LOADER_TARGET_EZP)
    return new EZPLoaderPlatform();
#else
#error "No loader platform defined"
    return nullptr;
#endif
}
