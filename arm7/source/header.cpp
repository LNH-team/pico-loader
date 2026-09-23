#include "common.h"
#include <stddef.h>
#include <string>
#include "header.h"

extern "C" void _start();
extern u8 __bss_start[];
extern u8 __bss_size[];

// Makefile.arm7 always defines these. The defaults are for compiling this file
// without it, for example in an editor's code model.
#ifndef GIT_HASH
#define GIT_HASH
#endif
#ifndef GIT_TAG
#define GIT_TAG
#endif
#ifndef GIT_AT_TAG
#define GIT_AT_TAG 0
#endif
#ifndef GIT_DIRTY
#define GIT_DIRTY 0
#endif
#ifndef PICO_LOADER_PLATFORM
#define PICO_LOADER_PLATFORM
#endif

static constexpr int parseHexDigit(char hexDigit)
{
    if (hexDigit >= '0' && hexDigit <= '9')
    {
        return hexDigit - '0';
    }
    else if (hexDigit >= 'a' && hexDigit <= 'f')
    {
        return 10 + hexDigit - 'a';
    }
    else if (hexDigit >= 'A' && hexDigit <= 'F')
    {
        return 10 + hexDigit - 'A';
    }
    else
    {
        return -1;
    }
}

// Returns -1 unless str is a number from 0 to 255.
static constexpr int parseVersionNumber(const std::string& str)
{
    if (str.empty())
    {
        return -1;
    }

    int value = 0;
    for (char c : str)
    {
        if (c < '0' || c > '9')
        {
            return -1;
        }

        value = value * 10 + (c - '0');
        if (value > 255)
        {
            return -1;
        }
    }
    return value;
}

// A tag that is not vX.Y.Z, such as a prerelease, or a hash that is not 40 hex
// digits leaves those fields zero instead of failing the build.
static consteval pload_version_info_t createVersionInfo()
{
    pload_version_info_t info { .platform = STRINGIFY(PICO_LOADER_PLATFORM) };

    std::string tag = STRINGIFY(GIT_TAG);
    if (tag.size() > 1 && tag[0] == 'v')
    {
        std::string version = tag.substr(1);
        size_t majorEnd = version.find('.');
        size_t minorEnd = majorEnd == std::string::npos ? std::string::npos : version.find('.', majorEnd + 1);
        if (minorEnd != std::string::npos)
        {
            int major = parseVersionNumber(version.substr(0, majorEnd));
            int minor = parseVersionNumber(version.substr(majorEnd + 1, minorEnd - majorEnd - 1));
            int patch = parseVersionNumber(version.substr(minorEnd + 1));
            if (major >= 0 && minor >= 0 && patch >= 0)
            {
                info.versionMajor = (u8)major;
                info.versionMinor = (u8)minor;
                info.versionPatch = (u8)patch;
            }
        }
    }

    std::string hash = STRINGIFY(GIT_HASH);
    bool hashValid = hash.size() == 40;
    for (size_t i = 0; hashValid && i < hash.size(); i++)
    {
        hashValid = parseHexDigit(hash[i]) >= 0;
    }

    if (hashValid)
    {
        for (int i = 0; i < 20; i++)
        {
            info.commitHash[i] = (u8)((parseHexDigit(hash[2 * i]) << 4) | parseHexDigit(hash[2 * i + 1]));
        }
    }

    info.versionFlags = (GIT_AT_TAG ? PICO_LOADER_VERSION_FLAGS_COMMIT_AT_TAG : 0)
        | (GIT_DIRTY ? PICO_LOADER_VERSION_FLAGS_DIRTY : 0);

    return info;
}

static constexpr pload_version_info_t sVersionInfo = createVersionInfo();

// Launchers use these offsets from their own copy of picoLoader7.h.
static_assert(offsetof(pload_header7_t, apiVersion) == 10);
static_assert(offsetof(pload_header7_t, loadParams) == 12);
static_assert(offsetof(pload_header7_t, v2) == 784);
static_assert(offsetof(pload_header7_t, v3) == 1040);
static_assert(offsetof(pload_header7_t, v4) == 1044);
static_assert(sizeof(pload_version_info_t) == 40);

// On the DSi and 3DS, dspico-wrfuxxed starts arm7 at the first byte of this header
// (mov pc, #0x06000000), so every word before _start is executed. That only works for
// words whose condition fails there, like zeros and pointers into vram, which is why
// v4 points to the version information instead of holding it.
[[gnu::section(".crt0")]]
[[gnu::used]]
pload_header7_t gLoaderHeader
{
    .entryPoint = (void*)&_start,
    .apiVersion = PICO_LOADER_API_VERSION,
    .v4 =
    {
        .versionInfo = &sVersionInfo
    }
};
