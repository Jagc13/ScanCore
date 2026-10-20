#include "scancore/DeviceManager.hpp"
#include "scancore/ScannerService.hpp"
#include "scancore/SerialConnection.hpp"

#include <chrono>
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>

int main(int argc, char** argv)
{
    try
    {
        scancore::DeviceManager manager;
        const auto devices = manager.discoverDevices();

        std::cout << "=== ScanCore Device Discovery ===\n";

        for (const auto& device : devices)
        {
            std::cout << device.productName
                      << " : " << device.portName << '\n';
        }

        std::string port;

        if (argc > 1)
        {
            port = argv[1];
        }
        else
        {
            for (const auto& device : devices)
            {
                if (device.productName.find("Barcode Scanner")
                        != std::string::npos &&
                    !device.portName.empty())
                {
                    port = device.portName;
                    break;
                }
            }
        }

        if (port.empty())
        {
            std::cerr << "No scanner selected.\n";
            return 1;
        }

        const auto baud = argc > 2
            ? static_cast<std::uint32_t>(std::stoul(argv[2]))
            : 9600u;

        scancore::SerialConnection serial(port, baud);
        scancore::ScannerService scanner(serial);

        scanner.setScanCallback([](const scancore::ScanEvent& event)
        {
            const auto time =
                std::chrono::system_clock::to_time_t(event.timestamp);

            std::cout << "ScanEvent { barcode: "
                      << event.barcode
                      << ", timestamp: "
                      << std::put_time(
                             std::localtime(&time),
                             "%Y-%m-%d %H:%M:%S")
                      << " }\n";
        });

        scanner.start();

        std::cout << "Listening on " << port
                  << " at " << baud << " baud.\n";

        std::cout << "Press Enter to stop scanning.\n";

        // Main thread remains free while scanner reads.
        std::cin.get();

        scanner.stop();

        std::cout << "Scanner stopped.\n";
    }
    catch (const std::exception& error)
    {
        std::cerr << "ScanCore error: "
                  << error.what() << '\n';
        return 1;
    }
}