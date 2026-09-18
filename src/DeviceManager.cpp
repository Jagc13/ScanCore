#include "scancore/DeviceManager.hpp"

#ifdef _WIN32

#include <windows.h>
#include <setupapi.h>
#include <devguid.h>

#include <cstdint>
#include <iostream>
#include <regex>
#include <string>
#include <utility>
#include <vector>

namespace
{

std::string getDeviceProperty(
    HDEVINFO deviceInfoSet,
    SP_DEVINFO_DATA& deviceInfoData,
    DWORD property)
{
    DWORD requiredSize = 0;

    SetupDiGetDeviceRegistryPropertyA(
        deviceInfoSet,
        &deviceInfoData,
        property,
        nullptr,
        nullptr,
        0,
        &requiredSize
    );

    if (requiredSize == 0)
    {
        return {};
    }

    std::vector<BYTE> buffer(requiredSize);

    if (!SetupDiGetDeviceRegistryPropertyA(
            deviceInfoSet,
            &deviceInfoData,
            property,
            nullptr,
            buffer.data(),
            static_cast<DWORD>(buffer.size()),
            nullptr))
    {
        return {};
    }

    return reinterpret_cast<const char*>(buffer.data());
}

std::string extractComPort(const std::string& friendlyName)
{
    const std::regex pattern(R"(\((COM\d+)\))");
    std::smatch match;

    if (std::regex_search(friendlyName, match, pattern))
    {
        return match[1].str();
    }

    return {};
}

std::uint16_t extractHexId(
    const std::string& hardwareId,
    const std::string& prefix)
{
    const auto position = hardwareId.find(prefix);

    if (position == std::string::npos)
    {
        return 0;
    }

    const auto start = position + prefix.length();

    if (start + 4 > hardwareId.length())
    {
        return 0;
    }

    try
    {
        return static_cast<std::uint16_t>(
            std::stoul(hardwareId.substr(start, 4), nullptr, 16)
        );
    }
    catch (...)
    {
        return 0;
    }
}

} // anonymous namespace


namespace scancore
{

std::vector<DeviceInfo> DeviceManager::discoverDevices()
{
    std::vector<DeviceInfo> devices;

    HDEVINFO deviceInfoSet = SetupDiGetClassDevsA(
        &GUID_DEVCLASS_PORTS,
        nullptr,
        nullptr,
        DIGCF_PRESENT
    );

    if (deviceInfoSet == INVALID_HANDLE_VALUE)
    {
        std::cerr << "Failed to retrieve Windows port devices.\n";
        return devices;
    }

    SP_DEVINFO_DATA deviceInfoData{};
    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    DWORD index = 0;

    while (SetupDiEnumDeviceInfo(
        deviceInfoSet,
        index,
        &deviceInfoData))
    {
        const std::string friendlyName =
            getDeviceProperty(
                deviceInfoSet,
                deviceInfoData,
                SPDRP_FRIENDLYNAME
            );

        const std::string hardwareId =
            getDeviceProperty(
                deviceInfoSet,
                deviceInfoData,
                SPDRP_HARDWAREID
            );

        const std::string manufacturer =
            getDeviceProperty(
                deviceInfoSet,
                deviceInfoData,
                SPDRP_MFG
            );

        if (!friendlyName.empty())
        {
            DeviceInfo device;

            device.productName = friendlyName;
            device.manufacturer = manufacturer;

            device.portName =
                extractComPort(friendlyName);

            device.vendorId =
                extractHexId(hardwareId, "VID_");

            device.productId =
                extractHexId(hardwareId, "PID_");

            if (!device.portName.empty())
            {
                device.devicePath =
                    "\\\\.\\" + device.portName;
            }

            // For now this means Windows reports the device as present.
            // Later we'll change this to represent an actual open connection.
            device.connected = true;

            devices.push_back(std::move(device));
        }

        ++index;
    }

    SetupDiDestroyDeviceInfoList(deviceInfoSet);

    return devices;
}

} // namespace scancore


#else

namespace scancore
{

std::vector<DeviceInfo> DeviceManager::discoverDevices()
{
    return {};
}

} // namespace scancore

#endif