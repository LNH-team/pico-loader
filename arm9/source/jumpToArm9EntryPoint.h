#pragma once

/// @brief Clears all cpu registers and jumps to the specified \p arm9EntryPoint.
/// @param arm9EntryPoint The arm9 entry point to jump to.
extern "C" void jumpToArm9EntryPoint(void* arm9EntryPoint);
