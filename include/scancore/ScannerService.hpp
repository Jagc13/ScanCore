#pragma once

#include "scancore/BarcodeCaptureEngine.hpp"
#include "scancore/IScannerConnection.hpp"

#include <atomic>
#include <functional>
#include <mutex>
#include <thread>

namespace scancore
{

class ScannerService
{
public:
    using ScanCallback = std::function<void(const ScanEvent&)>;

    explicit ScannerService(IScannerConnection& connection);
    ~ScannerService();

    ScannerService(const ScannerService&) = delete;
    ScannerService& operator=(const ScannerService&) = delete;

    void start();
    void stop();

    bool isRunning() const noexcept;

    void setScanCallback(ScanCallback callback);

private:
    void workerLoop();

    IScannerConnection& connection_;
    BarcodeCaptureEngine captureEngine_;

    std::thread worker_;
    std::atomic<bool> running_{false};

    std::mutex callbackMutex_;
    ScanCallback callback_;
};

} // namespace scancore