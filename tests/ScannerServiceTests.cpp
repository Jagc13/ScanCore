#include "scancore/ScannerService.hpp"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>

using namespace std::chrono_literals;

class FakeScannerConnection : public scancore::IScannerConnection
{
public:
    std::size_t read(char* buffer, std::size_t capacity) override
    {
        std::this_thread::sleep_for(10ms);

        if (!sent_.exchange(true))
        {
            const std::string barcode = "123456789\n";
            const auto count = std::min(capacity, barcode.size());

            std::memcpy(buffer, barcode.data(), count);
            return count;
        }

        return 0;
    }

    void reset()
    {
        sent_.store(false);
    }

private:
    std::atomic<bool> sent_{false};
};

int main()
{
    FakeScannerConnection connection;
    scancore::ScannerService scanner(connection);

    std::atomic<int> scanCount{0};

    scanner.setScanCallback(
        [&](const scancore::ScanEvent& event)
        {
            assert(event.barcode == "123456789");
            scanCount.fetch_add(1);
        });

    scanner.start();

    // Allow the worker to receive the fake scan.
    std::this_thread::sleep_for(100ms);

    assert(scanner.isRunning());
    assert(scanCount.load() == 1);

    scanner.stop();

    assert(!scanner.isRunning());

    // Verify restarting the scanner.
    connection.reset();
    scanner.start();

    std::this_thread::sleep_for(100ms);

    assert(scanCount.load() == 2);

    scanner.stop();

    assert(!scanner.isRunning());
}