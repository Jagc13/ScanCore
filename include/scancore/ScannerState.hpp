#pragma once

namespace scancore
{

enum class ScannerState
{
    Disconnected,
    Ready,
    Scanning,
    Error
};
inline const char* toString(ScannerState state)
{
    switch (state)
    {
        case ScannerState::Disconnected:
            return "Disconnected";

        case ScannerState::Ready:
            return "Ready";

        case ScannerState::Scanning:
            return "Scanning";

        case ScannerState::Error:
            return "Error";
    }

    return "Unknown";
}

} // namespace scancore