#include "scancore/ScannerService.hpp"

#include <array>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace scancore
{

ScannerService::ScannerService(IScannerConnection& connection)
    : connection_(connection)
{
}

ScannerService::~ScannerService()
{
    stop();
}

void ScannerService::start()
{
    if (running_.load())
    {
        return;
    }

    // Clean up a previous worker that has already finished.
    if (worker_.joinable())
    {
        worker_.join();
    }

    running_.store(true);

    try
    {
        worker_ = std::thread(&ScannerService::workerLoop, this);
    }
    catch (...)
    {
        running_.store(false);
        throw;
    }
}

void ScannerService::stop()
{
    running_.store(false);

    if (worker_.joinable())
    {
        worker_.join();
    }
}

bool ScannerService::isRunning() const noexcept
{
    return running_.load();
}

void ScannerService::setScanCallback(ScanCallback callback)
{
    std::lock_guard<std::mutex> lock(callbackMutex_);
    callback_ = std::move(callback);
}

void ScannerService::workerLoop()
{
    try
    {
        std::array<char, 256> buffer{};

        while (running_.load())
        {
            const auto count =
                connection_.read(buffer.data(), buffer.size());

            if (!running_.load())
            {
                break;
            }

            if (count == 0)
            {
                continue;
            }

            const auto events = captureEngine_.feed(
                std::string_view(buffer.data(), count));

            for (const auto& event : events)
            {
                if (!running_.load())
                {
                    break;
                }

                ScanCallback callback;

                {
                    std::lock_guard<std::mutex> lock(callbackMutex_);
                    callback = callback_;
                }

                if (callback)
                {
                    callback(event);
                }
            }
        }
    }
    catch (const std::exception& error)
    {
        std::cerr << "Scanner worker error: "
                  << error.what() << '\n';
    }
    catch (...)
    {
        std::cerr << "Unknown scanner worker error\n";
    }

    running_.store(false);
}

} // namespace scancore