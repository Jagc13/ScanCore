#pragma once

#include "scancore/ScannerService.hpp"
#include "scancore/SerialConnection.hpp"

#include <QMainWindow>
#include <QString>

#include <memory>

class QLabel;
class QPushButton;
class QTableWidget;
class QTimer;

class MainWindow : public QMainWindow
{
public:
    explicit MainWindow(
        const QString& port,
        QWidget* parent = nullptr);

    ~MainWindow() override;

private:
    void updateState();
    void addBarcode(const QString& barcode,
                    const QString& timestamp);

    QLabel* statusLabel_;
    QLabel* barcodeLabel_;

    QPushButton* enableButton_;
    QPushButton* disableButton_;
    QPushButton* startButton_;
    QPushButton* stopButton_;

    QTableWidget* historyTable_;
    QTimer* statusTimer_;

    // ScannerService must be destroyed before SerialConnection.
    std::unique_ptr<scancore::SerialConnection> serial_;
    std::unique_ptr<scancore::ScannerService> scanner_;
};