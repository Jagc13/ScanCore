#include "scancore/BarcodeCaptureEngine.hpp"
#include <chrono>
namespace scancore {
std::vector<ScanEvent> BarcodeCaptureEngine::feed(std::string_view bytes) {
    std::vector<ScanEvent> events;
    for (unsigned char byte : bytes) {
        if (byte == '\r' || byte == '\n') {
            if (!discarding_ && !pending_.empty())
                events.push_back({std::move(pending_), std::chrono::system_clock::now()});
            pending_.clear();
            discarding_ = false;
        } else if (!discarding_) {
            // Accept printable ASCII and UTF-8 bytes; reject controls and DEL.
            if (byte < 0x20 || byte == 0x7f) {
                pending_.clear();
                discarding_ = true;
            } else if (pending_.size() == maxLength) {
                pending_.clear();
                discarding_ = true;
            } else {
                pending_.push_back(static_cast<char>(byte));
            }
        }
    }
    return events;
}
}
