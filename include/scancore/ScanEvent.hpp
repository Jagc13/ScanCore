#pragma once
#include <chrono>
#include <string>
namespace scancore {
struct ScanEvent {
    std::string barcode;
    std::chrono::system_clock::time_point timestamp;
};
}
