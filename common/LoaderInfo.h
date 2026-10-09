#pragma once

struct loader_info_t
{
    u16 clusterShift;
    u16 picoLoaderBootDrive;
    u32 database;
    u32 clusterMap9[10];
    u32 clusterMap7[16];
    char launcherPath[256];
};

static_assert(sizeof(loader_info_t) == 0x170);
