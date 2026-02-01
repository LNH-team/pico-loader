#pragma once

/// @brief Class containing helper functions for manipulating ARM or Thumb assembly.
class ArmHelper
{
public:
    ArmHelper() = delete;

    /// @brief Check if an ARM instruction is an unconditional bl call.
    /// @param instruction The ARM instruction to check.
    /// @return True if the instruction is an unconditional bl call, false otherwise.
    static bool IsArmUnconditionalBl(u32 instruction)
    {
        return (instruction >> 24) == 0xEB;
    }

    /// @brief Get the offset of an ARM call (bl or blx).
    /// @param callInstruction The ARM call instruction.
    /// @return The offset to the destination function. +1 if Thumb.
    static s32 GetArmCallOffset(u32 callInstruction)
    {
        return 8 + ((int)((callInstruction & 0xFFFFFF) << 8) >> 6)
            + ((callInstruction >> 24) == 0xFA ? 1 : 0);
    }

    /// @brief Get the address of the callee in an ARM call (bl or blx).
    /// @param instructionPointer Pointer to the ARM call instruction.
    /// @return The address of the function being called. +1 if Thumb.
    static u32 GetArmCallAddress(const void* instructionPointer)
    {
        auto pc = (const u32*)((u32)instructionPointer & ~3);
        return (u32)pc + GetArmCallOffset(*pc);
    }

    /// @brief Create an ARM call instruction (bl or blx).
    /// @param instructionAddress The address where the instruction is to be placed.
    /// @param targetAddress The address of the function to call. +1 if Thumb.
    /// @return The ARM call instruction to call the target.
    static u32 MakeArmCall(u32 instructionAddress, u32 targetAddress)
    {
        u32 offset = targetAddress - instructionAddress - 8;
        return ((targetAddress & 1) ? 0xFA000000 : 0xEB000000) | ((offset >> 2) & 0xFFFFFF);
    }

    /// @brief Get the offset of a Thumb call (bl or blx).
    /// @param callInstruction1 The first Thumb call instruction.
    /// @param callInstruction2 The second Thumb call instruction.
    /// @return The offset to the destination function. +1 if Thumb.
    static s32 GetThumbCallOffset(u16 callInstruction1, u16 callInstruction2)
    {
        return 4 + ((int)((((callInstruction1 & 0x7FF) << 11) | (callInstruction2 & 0x7FF)) << 10) >> 9)
            + ((callInstruction2 >> 11) == 0x1F ? 1 : 0);
    }

    /// @brief Get the address of the callee in an Thumb call (bl or blx).
    /// @param instructionPointer Pointer to the first Thumb call instruction.
    /// @return The offset to the destination function. +1 if Thumb.
    static u32 GetThumbCallAddress(const void* instructionPointer)
    {
        auto pc = (const u16*)((u32)instructionPointer & ~1);
        return (u32)pc + GetThumbCallOffset(pc[0], pc[1]);
    }
};
