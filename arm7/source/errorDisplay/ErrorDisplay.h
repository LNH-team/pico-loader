#pragma once

/// @brief Class for displaying critical errors on screen.
class ErrorDisplay
{
public:
    /// @brief Formats and sends a warning message to the arm9 to display it on screen.
    ///        Pressing the A button will dismiss the warning and make this function return.
    /// @param errorFormat The error message to format.
    void PrintWarning(const char* errorFormat, ...);

    /// @brief Formats and sends a critical error message to the arm9 to display it on screen.
    /// @note This function does not return.
    /// @param errorFormat The error message to format.
    void PrintError(const char* errorFormat, ...);
};
