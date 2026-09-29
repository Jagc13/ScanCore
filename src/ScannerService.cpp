#include "scancore/ScannerService.hpp"

#include <array>
#include <iostream>
#include <string_view>

namespace scancore
{

ScannerService::ScannerService(IScannerConnection& connection)
    : connection_(connection)
{
}

void ScannerService::run()
{
    std::array<char, 256> buffer{};

    std::cout << "Waiting for barcode scans...\n";

    while (true)
    {
        const std::size_t bytesRead =
            connection_.read(buffer.data(), buffer.size());

        if (bytesRead == 0)
        {
            continue;
        }

        const std::string_view bytes(
            buffer.data(),
            bytesRead);

        const auto events =
            captureEngine_.feed(bytes);

        for (const auto& event : events)
        {
            std::cout << "[SCAN] "
                      << event.barcode
                      << '\n';
        }
    }
}

} // namespace scancore