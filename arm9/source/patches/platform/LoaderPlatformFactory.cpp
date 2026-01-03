#include "common.h"
#include "patches/platform/LoaderPlatform.h"
#include "patches/platform/dspico/DSPicoLoaderPlatform.h"
#include "patches/platform/isnitro/IsNitroLoaderPlatform.h"
#include "patches/platform/m3ds/M3DSLoaderPlatform.h"
#include "patches/platform/r4/R4LoaderPlatform.h"
#include "patches/platform/ace3ds/Ace3DSLoaderPlatform.h"
#include "patches/platform/g003/G003LoaderPlatform.h"
#include "patches/platform/melonds/MelonDSLoaderPlatform.h"
#include "patches/platform/dstt/DsttLoaderPlatform.h"
#include "patches/platform/ak2/Ak2LoaderPlatform.h"
#include "patches/platform/akrpg/AkRpgLoaderPlatform.h"
#include "patches/platform/r4idsn/R4iDsnLoaderPlatform.h"
#include "patches/platform/supercard/SuperCardLoaderPlatform.h"
#include "patches/platform/ezp/EzpLoaderPlatform.h"
#include "patches/platform/datel/DatelLoaderPlatform.h"
#include "patches/platform/stargate/StargateLoaderPlatform.h"
#include "patches/platform/supercardcf/SuperCardCFLoaderPlatform.h"
#include "patches/platform/mpcf/MPCFLoaderPlatform.h"
#include "patches/platform/m3cf/M3CFLoaderPlatform.h"
#include "patches/platform/mmcf/MMCFLoaderPlatform.h"
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
    return new DsttLoaderPlatform();
#elif defined(PICO_LOADER_TARGET_AK2)
    return new Ak2LoaderPlatform();
#elif defined(PICO_LOADER_TARGET_AKRPG)
    return new AkRpgLoaderPlatform();
#elif defined(PICO_LOADER_TARGET_R4iDSN)
    return new R4iDsnLoaderPlatform();
#elif defined(PICO_LOADER_TARGET_SUPERCARD)
    return new SuperCardLoaderPlatform();
#elif defined(PICO_LOADER_TARGET_EZP)
    return new EzpLoaderPlatform();
#elif defined(PICO_LOADER_TARGET_DATEL)
    return new DatelLoaderPlatform();
#elif defined(PICO_LOADER_TARGET_STARGATE)
    return new StargateLoaderPlatform();
#elif defined(PICO_LOADER_TARGET_SUPERCARDCF)
    return new SuperCardCFLoaderPlatform();
#elif defined(PICO_LOADER_TARGET_MPCF)
    return new MPCFLoaderPlatform();
#elif defined(PICO_LOADER_TARGET_M3CF)
    return new M3CFLoaderPlatform();
#elif defined(PICO_LOADER_TARGET_MMCF)
    return new MMCFLoaderPlatform();
#else
#error "No loader platform defined"
    return nullptr;
#endif
}
