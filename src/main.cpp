#include "scancore/DeviceManager.hpp"
#include "scancore/ScannerService.hpp"
#include "scancore/ScannerState.hpp"
#include "scancore/SerialConnection.hpp"

#include <chrono>
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <string>

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
            std::cerr << "No scanner found.\n";
            return 1;
        }

        const auto baud = argc > 2
            ? static_cast<std::uint32_t>(std::stoul(argv[2]))
            : 9600u;

        scancore::SerialConnection serial(port, baud);
        scancore::ScannerService scanner(serial);

        scanner.setScanCallback(
            [](const scancore::ScanEvent& event)
            {
                const auto time =
                    std::chrono::system_clock::to_time_t(
                        event.timestamp);

                std::cout << "\n[SCAN] "
                          << event.barcode
                          << " at "
                          << std::put_time(
                                 std::localtime(&time),
                                 "%Y-%m-%d %H:%M:%S")
                          << '\n';
            });

        int choice = 0;

        while (choice != 6)
        {
            std::cout << "\n=== ScanCore Scanner Control ===\n";
            std::cout << "Scanner State: "
                      << scancore::toString(scanner.getState())
                      << '\n';

            std::cout << "1. Enable Scanner\n";
            std::cout << "2. Disable Scanner\n";
            std::cout << "3. Start Reading\n";
            std::cout << "4. Stop Reading\n";
            std::cout << "5. Show Scanner State\n";
            std::cout << "6. Exit\n";
            std::cout << "Enter choice: ";

            if (!(std::cin >> choice))
            {
                std::cout << "Invalid input.\n";
                break;
            }

            try
            {
                switch (choice)
                {
                    case 1:
                        scanner.enable();
                        std::cout << "Scanner enabled.\n";
                        break;

                    case 2:
                        scanner.disable();
                        std::cout << "Scanner disabled.\n";
                        break;

                    case 3:
                        scanner.startReading();
                        std::cout << "Scanner reading started.\n";
                        break;

                    case 4:
                        scanner.stopReading();
                        std::cout << "Scanner reading stopped.\n";
                        break;

                    case 5:
                        std::cout << "Current State: "
                                  << scancore::toString(
                                         scanner.getState())
                                  << '\n';
                        break;

                    case 6:
                        scanner.stopReading();
                        std::cout << "Exiting ScanCore.\n";
                        break;

                    default:
                        std::cout << "Invalid choice.\n";
                        break;
                }
            }
            catch (const std::exception& error)
            {
                std::cerr << "Control error: "
                          << error.what() << '\n';
            }
        }

        scanner.stopReading();
    }
    catch (const std::exception& error)
    {
        std::cerr << "ScanCore error: "
                  << error.what() << '\n';
        return 1;
    }

    return 0;
}