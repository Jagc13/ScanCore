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
    startReading();
}

void ScannerService::stop()
{
    stopReading();
}

bool ScannerService::isRunning() const noexcept
{
    return running_.load();
}

ScannerState ScannerService::getState() const noexcept
{
    return state_.load();
}

void ScannerService::enable()
{
    if (state_.load() == ScannerState::Disconnected)
    {
        state_.store(ScannerState::Ready);
    }
}

void ScannerService::disable()
{
    stopReading();
    state_.store(ScannerState::Disconnected);
}

void ScannerService::startReading()
{
    const auto state = state_.load();

    if (state == ScannerState::Disconnected)
    {
        throw std::runtime_error("Scanner is disabled");
    }

    if (state == ScannerState::Error)
    {
        throw std::runtime_error("Scanner is in error state");
    }

    if (running_.load())
    {
        return;
    }

    // Join a previous worker before starting a new one.
    if (worker_.joinable())
    {
        worker_.join();
    }

    running_.store(true);
    state_.store(ScannerState::Scanning);

    try
    {
        worker_ = std::thread(&ScannerService::workerLoop, this);
    }
    catch (...)
    {
        running_.store(false);
        state_.store(ScannerState::Error);
        throw;
    }
}

void ScannerService::stopReading()
{
    running_.store(false);

    if (worker_.joinable())
    {
        worker_.join();
    }

    // Preserve an error or disabled state.
    if (state_.load() == ScannerState::Scanning)
    {
        state_.store(ScannerState::Ready);
    }
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

        state_.store(ScannerState::Error);
    }
    catch (...)
    {
        std::cerr << "Unknown scanner worker error\n";
        state_.store(ScannerState::Error);
    }

    running_.store(false);
}

} // namespace scancore