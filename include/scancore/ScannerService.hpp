#pragma once

#include "scancore/BarcodeCaptureEngine.hpp"
#include "scancore/IScannerConnection.hpp"

namespace scancore
{

class ScannerService
{
public:
    explicit ScannerService(IScannerConnection& connection);

    void run();

private:
    IScannerConnection& connection_;
    BarcodeCaptureEngine captureEngine_;
};

} // namespace scancore