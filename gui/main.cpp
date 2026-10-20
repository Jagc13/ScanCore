#include "MainWindow.hpp"

#include "scancore/DeviceManager.hpp"

#include <QApplication>
#include <QMessageBox>
#include <QString>

#include <exception>
#include <string>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    try
    {
        std::string port;

        if (argc > 1)
        {
            port = argv[1];
        }
        else
        {
            scancore::DeviceManager manager;
            const auto devices = manager.discoverDevices();

            for (const auto& device : devices)
            {
               if (device.productName.find("Barcode Scanner")
                      != std::string::npos &&
                   !device.portName.empty() &&
                       device.connected)
                {
                    port = device.portName;
                    break;
                }
            }
        }

        if (port.empty())
        {
            QMessageBox::critical(
                nullptr,
                "Scanner Not Found",
                "No barcode scanner detected.");

            return 1;
        }

        MainWindow window(QString::fromStdString(port));
        window.show();

        return app.exec();
    }
    catch (const std::exception& error)
    {
        QMessageBox::critical(
            nullptr, "ScanCore Error", error.what());

        return 1;
    }
}