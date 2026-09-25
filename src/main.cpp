#include "scancore/BarcodeCaptureEngine.hpp"
#include "scancore/DeviceManager.hpp"
#include "scancore/SerialConnection.hpp"
#include <array>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <string>
int main(int argc, char** argv) {
    try {
        scancore::DeviceManager manager;
        const auto devices = manager.discoverDevices();
        std::cout << "=== ScanCore Device Discovery ===\n";
        for (const auto& device : devices)
            std::cout << device.productName << " : " << device.portName << '\n';
        std::string port;
        if (argc > 1) port = argv[1];
        else {
            for (const auto& device : devices)
                if (device.productName.find("Barcode Scanner") != std::string::npos && !device.portName.empty()) {
                    port = device.portName;
                    break;
                }
        }
        if (port.empty()) {
            std::cerr << "No scanner selected. Pass its port, for example: scancore.exe COM3\n";
            return 1;
        }
        const auto baud = argc > 2 ? static_cast<std::uint32_t>(std::stoul(argv[2])) : 9600u;
        scancore::SerialConnection serial(port, baud);
        scancore::BarcodeCaptureEngine capture;
        std::cout << "Listening on " << port << " at " << baud << " baud. Press Ctrl+C to stop.\n";
        std::array<char, 256> buffer{};
        while (true) {
            const auto count = serial.read(buffer.data(), buffer.size());
            for (const auto& event : capture.feed({buffer.data(), count})) {
                const auto time = std::chrono::system_clock::to_time_t(event.timestamp);
                std::cout << "ScanEvent { barcode: " << event.barcode << ", timestamp: "
                          << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << " }\n";
            }
        }
    } catch (const std::exception& error) {
        std::cerr << "ScanCore error: " << error.what() << '\n';
        return 1;
    }
}
