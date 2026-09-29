#pragma once

#include <cstddef>

namespace scancore
{

class IScannerConnection
{
public:
    virtual ~IScannerConnection() = default;

    virtual std::size_t read(
        char* buffer,
        std::size_t capacity) = 0;
};

} // namespace scancore