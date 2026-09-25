#include "scancore/SerialConnection.hpp"
#include <stdexcept>
#include <utility>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <algorithm>
#include <system_error>
namespace {
void check(bool ok, const char* operation) {
    if (!ok) throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), operation);
}
}
namespace scancore {
struct SerialConnection::Impl {
    HANDLE handle = INVALID_HANDLE_VALUE;
    ~Impl() { if (handle != INVALID_HANDLE_VALUE) CloseHandle(handle); }
};
SerialConnection::SerialConnection(const std::string& port, std::uint32_t baud) : impl_(std::make_unique<Impl>()) {
    const std::string path = port.rfind("\\\\.\\", 0) == 0 ? port : "\\\\.\\" + port;
    impl_->handle = CreateFileA(path.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (impl_->handle == INVALID_HANDLE_VALUE)
        throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), "Open " + path);
    DCB state{};
    state.DCBlength = sizeof(state);
    check(GetCommState(impl_->handle, &state), "GetCommState");
    state.BaudRate = baud;
    state.ByteSize = 8;
    state.Parity = NOPARITY;
    state.StopBits = ONESTOPBIT;
    state.fBinary = TRUE;
    state.fDtrControl = DTR_CONTROL_ENABLE;
    state.fRtsControl = RTS_CONTROL_ENABLE;
    check(SetCommState(impl_->handle, &state), "SetCommState");
    COMMTIMEOUTS timeouts{};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 250;
    check(SetCommTimeouts(impl_->handle, &timeouts), "SetCommTimeouts");
}
SerialConnection::~SerialConnection() = default;
SerialConnection::SerialConnection(SerialConnection&&) noexcept = default;
SerialConnection& SerialConnection::operator=(SerialConnection&&) noexcept = default;
std::size_t SerialConnection::read(char* buffer, std::size_t capacity) {
    if (!impl_ || !buffer || !capacity) throw std::invalid_argument("Invalid serial read");
    DWORD count = 0;
    check(ReadFile(impl_->handle, buffer, static_cast<DWORD>(std::min<std::size_t>(capacity, MAXDWORD)), &count, nullptr), "ReadFile");
    return count;
}
}
#else
namespace scancore {
struct SerialConnection::Impl {};
SerialConnection::SerialConnection(const std::string&, std::uint32_t) { throw std::runtime_error("Serial capture is not implemented on this platform"); }
SerialConnection::~SerialConnection() = default;
SerialConnection::SerialConnection(SerialConnection&&) noexcept = default;
SerialConnection& SerialConnection::operator=(SerialConnection&&) noexcept = default;
std::size_t SerialConnection::read(char*, std::size_t) { throw std::runtime_error("Serial capture is not implemented on this platform"); }
}
#endif
