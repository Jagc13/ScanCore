#pragma once
#include "scancore/ScanEvent.hpp"
#include <string>
#include <string_view>
#include <vector>
namespace scancore {
class BarcodeCaptureEngine {
public:
    // Feed any number of bytes; a scan ends at CR, LF or CRLF.
    std::vector<ScanEvent> feed(std::string_view bytes);
private:
    static constexpr std::size_t maxLength = 4096;
    std::string pending_;
    bool discarding_ = false;
};
}
