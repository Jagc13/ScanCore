#include <iostream>

#include "scancore/DeviceManager.hpp"

#include <iomanip>
#include <iostream>

int main()
{
    scancore::DeviceManager manager;

    const auto devices = manager.discoverDevices();

    std::cout << "=== ScanCore Device Discovery ===\n\n";

    if (devices.empty())
    {
        std::cout << "No serial devices found.\n";
        return 0;
    }

    for (const auto& device : devices)
    {
        std::cout << "Device\n";
        std::cout << "----------------------------\n";

        std::cout << "Name: "
                  << device.productName << '\n';

        std::cout << "Port: "
                  << device.portName << '\n';

        std::cout << "VID:  0x"
                  << std::hex
                  << std::uppercase
                  << std::setw(4)
                  << std::setfill('0')
                  << device.vendorId
                  << '\n';

        std::cout << "PID:  0x"
                  << std::setw(4)
                  << device.productId
                  << '\n';

        std::cout << std::dec;

        std::cout << "Status: "
                  << (device.connected
                          ? "CONNECTED"
                          : "DISCONNECTED")
                  << "\n\n";
    }

    return 0;
}