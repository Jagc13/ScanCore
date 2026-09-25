#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
namespace scancore {
class SerialConnection {
public:
    explicit SerialConnection(const std::string& port, std::uint32_t baud = 9600);
    ~SerialConnection();
    SerialConnection(const SerialConnection&) = delete;
    SerialConnection& operator=(const SerialConnection&) = delete;
    SerialConnection(SerialConnection&&) noexcept;
    SerialConnection& operator=(SerialConnection&&) noexcept;
    // Returns zero on a read timeout, otherwise number of bytes read.
    std::size_t read(char* buffer, std::size_t capacity);
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
}
