#pragma once

/// @brief Interface for an autoload address adjuster.
class IAutoloadAdjuster
{
public:
    virtual ~IAutoloadAdjuster() { }

    /// @brief Adjust an initial address (pre-autoload) to its final location after autoload.
    /// @param initialAddress The address to adjust for autoloading.
    /// @return The final location of the address.
    virtual u32 AdjustInitialToFinal(u32 initialAddress) const = 0;

    /// @brief Adjust a final address (post-autoload) to its initial location before autoload.
    /// @param finalAddress The address to adjust for autoloading.
    /// @return The initial location of the address.
    virtual u32 AdjustFinalToInitial(u32 finalAddress) const = 0;

protected:
    IAutoloadAdjuster() { }
};
