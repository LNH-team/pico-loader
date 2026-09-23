#include "common.h"
#include "header.h"

extern "C" void _start();
extern u8 __bss_start[];
extern u8 __bss_size[];

[[gnu::section(".crt0")]]
[[gnu::used]]
pload_header7_t gLoaderHeader
{
    .entryPoint = (void*)&_start,
    .apiVersion = PICO_LOADER_API_VERSION,
    .v4 =
    {
        .gameLanguage = PLOAD_GAME_LANGUAGE_AUTO
    }
};