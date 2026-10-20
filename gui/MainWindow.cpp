#include "MainWindow.hpp"

#include "scancore/ScannerState.hpp"

#include <QDateTime>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QMetaObject>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include <exception>

MainWindow::MainWindow(
    const QString& port,
    QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("ScanCore - Barcode Scanner");
    resize(800, 500);

    auto* central = new QWidget(this);
    auto* layout = new QVBoxLayout(central);

    statusLabel_ = new QLabel("Scanner State: Ready");
    layout->addWidget(statusLabel_);

    auto* buttons = new QHBoxLayout();

    enableButton_ = new QPushButton("Enable");
    disableButton_ = new QPushButton("Disable");
    startButton_ = new QPushButton("Start Reading");
    stopButton_ = new QPushButton("Stop Reading");

    buttons->addWidget(enableButton_);
    buttons->addWidget(disableButton_);
    buttons->addWidget(startButton_);
    buttons->addWidget(stopButton_);

    layout->addLayout(buttons);

    barcodeLabel_ = new QLabel("Latest Barcode: None");
    layout->addWidget(barcodeLabel_);

    historyTable_ = new QTableWidget(0, 2);
    historyTable_->setHorizontalHeaderLabels(
        {"Barcode", "Timestamp"});

    historyTable_->horizontalHeader()
        ->setSectionResizeMode(QHeaderView::Stretch);

    layout->addWidget(historyTable_);
    setCentralWidget(central);

    // Open the physical scanner.
    serial_ = std::make_unique<scancore::SerialConnection>(
        port.toStdString(), 9600);

    scanner_ = std::make_unique<scancore::ScannerService>(
        *serial_);

    // Barcode callback executes on the scanner worker thread.
    scanner_->setScanCallback(
        [this](const scancore::ScanEvent& event)
        {
            const QString barcode =
                QString::fromStdString(event.barcode);

            const auto milliseconds =
                std::chrono::duration_cast<
                    std::chrono::milliseconds>(
                    event.timestamp.time_since_epoch()).count();

            const QString timestamp =
                QDateTime::fromMSecsSinceEpoch(milliseconds)
                    .toString("yyyy-MM-dd HH:mm:ss");

            // Queue widget updates on the GUI thread.
            QMetaObject::invokeMethod(
                this,
                [this, barcode, timestamp]()
                {
                    addBarcode(barcode, timestamp);
                },
                Qt::QueuedConnection);
        });

    connect(enableButton_, &QPushButton::clicked,
            this, [this]()
    {
        scanner_->enable();
        updateState();
    });

    connect(disableButton_, &QPushButton::clicked,
            this, [this]()
    {
        scanner_->disable();
        updateState();
    });

    connect(startButton_, &QPushButton::clicked,
            this, [this]()
    {
        try
        {
            scanner_->startReading();
        }
        catch (const std::exception& error)
        {
            QMessageBox::warning(
                this, "Scanner Error", error.what());
        }

        updateState();
    });

    connect(stopButton_, &QPushButton::clicked,
            this, [this]()
    {
        scanner_->stopReading();
        updateState();
    });

    // Periodically reflect worker-thread state changes.
    statusTimer_ = new QTimer(this);

    connect(statusTimer_, &QTimer::timeout,
            this, [this]()
    {
        updateState();
    });

    statusTimer_->start(200);
    updateState();
}

MainWindow::~MainWindow()
{
    statusTimer_->stop();

    // Wait for the worker before destroying Qt widgets.
    scanner_->stopReading();
    scanner_->setScanCallback({});
}

void MainWindow::updateState()
{
    const auto state = scanner_->getState();

    statusLabel_->setText(
        "Scanner State: " +
        QString::fromUtf8(scancore::toString(state)));

    using scancore::ScannerState;

    enableButton_->setEnabled(
        state == ScannerState::Disconnected);

    disableButton_->setEnabled(
        state == ScannerState::Ready ||
        state == ScannerState::Scanning);

    startButton_->setEnabled(
        state == ScannerState::Ready);

    stopButton_->setEnabled(
        state == ScannerState::Scanning);
}

void MainWindow::addBarcode(
    const QString& barcode,
    const QString& timestamp)
{
    barcodeLabel_->setText("Latest Barcode: " + barcode);

    const int row = historyTable_->rowCount();

    historyTable_->insertRow(row);

    historyTable_->setItem(
        row, 0, new QTableWidgetItem(barcode));

    historyTable_->setItem(
        row, 1, new QTableWidgetItem(timestamp));

    historyTable_->scrollToBottom();
}