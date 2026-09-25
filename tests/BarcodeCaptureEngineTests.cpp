#include "scancore/BarcodeCaptureEngine.hpp"
#include <cassert>
#include <chrono>
int main() {
    scancore::BarcodeCaptureEngine engine;
    assert(engine.feed("123").empty());
    auto events = engine.feed("45\r\nABC-9\n");
    assert(events.size() == 2 && events[0].barcode == "12345" && events[1].barcode == "ABC-9");
    assert(events[0].timestamp <= std::chrono::system_clock::now());
    assert(engine.feed("\n\r").empty());
    assert(engine.feed("bad\x01skip\r").empty());
    events = engine.feed("OK\r");
    assert(events.size() == 1 && events[0].barcode == "OK");
}
