#pragma once

/// @brief Interface for patch code implementing routines to send a SD command
class ISuperCardSendSdCommandPatchCode
{
protected:
    ISuperCardSendSdCommandPatchCode() { }

public:
    /// @brief Gets a pointer to the send SD command function in the patch code.
    /// @return The pointer to the send SD command.
    virtual const void* GetSendSdCommandFunction() const = 0;
};
