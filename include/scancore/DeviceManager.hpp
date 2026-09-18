#pragma once

#include "DeviceInfo.hpp"

#include <vector>

namespace scancore
{

class DeviceManager
{
public:
    std::vector<DeviceInfo> discoverDevices();
};

} // namespace scancore