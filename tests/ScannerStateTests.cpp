#include "scancore/ScannerService.hpp"
#include "scancore/ScannerState.hpp"

#include <cassert>
#include <chrono>
#include <cstddef>
#include <thread>

using namespace std::chrono_literals;

class FakeConnection : public scancore::IScannerConnection
{
public:
    std::size_t read(char*, std::size_t) override
    {
        std::this_thread::sleep_for(10ms);
        return 0;
    }
};

int main()
{
    FakeConnection connection;
    scancore::ScannerService scanner(connection);

    using scancore::ScannerState;

    // 1. Initially Ready
    assert(scanner.getState() == ScannerState::Ready);

    // 2. Start reading
    scanner.startReading();
    assert(scanner.getState() == ScannerState::Scanning);
    assert(scanner.isRunning());

    // 3. Stop reading
    scanner.stopReading();
    assert(scanner.getState() == ScannerState::Ready);
    assert(!scanner.isRunning());

    // 4. Disable scanner
    scanner.disable();
    assert(scanner.getState() == ScannerState::Disconnected);

    // 5. Enable scanner again
    scanner.enable();
    assert(scanner.getState() == ScannerState::Ready);

    // 6. Restart scanning
    scanner.startReading();
    assert(scanner.getState() == ScannerState::Scanning);

    scanner.stopReading();
    assert(scanner.getState() == ScannerState::Ready);

    // 7. Disable while scanning
    scanner.startReading();
    scanner.disable();

    assert(scanner.getState() == ScannerState::Disconnected);
    assert(!scanner.isRunning());

    return 0;
}