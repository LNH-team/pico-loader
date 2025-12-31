#pragma once
#include "common.h"

namespace CompactFlash {

	struct CF_REGISTERS {
		u32 data;
		u32 status;
		u32 command;
		u32 error;
		u32 sectorCount;
		u32 lba1;
		u32 lba2;
		u32 lba3;
		u32 lba4;
	};

}