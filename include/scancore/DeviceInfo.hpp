#pragma once

#include <cstdint>
#include <string>

namespace scancore
{

struct DeviceInfo
{
    std::uint16_t vendorId{};
    std::uint16_t productId{};

    std::string manufacturer;
    std::string productName;
    std::string portName;
    std::string devicePath;

    bool connected{false};
};

} // namespace scancore