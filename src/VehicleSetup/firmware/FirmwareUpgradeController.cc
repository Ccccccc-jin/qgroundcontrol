#include <QStandardPaths>
#include <QDebug>

#include "FirmwareUpgradeController.h"
#include "FirmwareUpgraderInterface.h"
#include "QGCQFileDialog.h"


FirmwareUpgradeController::FirmwareUpgradeController(void)
    : _fwUpgrader(std::move(FirmwareUpgrader::instance()))
{
    _connectToFirmwareUpgrader();
}


FirmwareUpgradeController::~FirmwareUpgradeController(void)
{  }


bool FirmwareUpgradeController::deviceAvailable(void) const
{
    return _fwUpgrader->deviceAvailable();
}


bool FirmwareUpgradeController::checksumEnabled(void) const
{
    return _fwUpgrader->checksumEnabled();
}


void FirmwareUpgradeController::_connectToFirmwareUpgrader(void) {
    auto fwUpgraderPtr = _fwUpgrader.get();

    QObject::connect(fwUpgraderPtr, &FirmwareUpgrader::statusMessageReceived,
                     this,          &FirmwareUpgradeController::infoMsgReceived);

    QObject::connect(fwUpgraderPtr, &FirmwareUpgrader::errorMessageReceived,
                     this,          &FirmwareUpgradeController::errorMsgReceived);

    QObject::connect(fwUpgraderPtr, &FirmwareUpgrader::flasherProgressChanged,
                     this,          &FirmwareUpgradeController::flasherProgressChanged);
}


void FirmwareUpgradeController::_startPolling(void)
{
    _pollingTimer.setInterval(1); // 1 sec
    _pollingTimer.setSingleShot(false);

    QObject::connect(&_pollingTimer, &QTimer::timeout,
                     this,           &FirmwareUpgradeController::_onTimeout);

    _pollingTimer.start();
}


void FirmwareUpgradeController::_onTimeout(void)
{
    if (!_fwUpgrader->deviceAvailable()) {
        return;
    }

    _pollingTimer.stop();
    emit deviceFound();
}


void FirmwareUpgradeController::enableChecksum(bool checksumEnabled)
{
    _fwUpgrader-> enableChecksum(checksumEnabled);
}


void FirmwareUpgradeController::searchDevice(void)
{
    qInfo() << "Polling started...";
    _startPolling();
}


void FirmwareUpgradeController::flash(QString const& firmwareFilename)
{
    auto image = new FirmwareImage();
    image->setBinFilename(firmwareFilename);
    _fwUpgrader->flash(image);
}


void FirmwareUpgradeController::cancelFlashing(void)
{
    _fwUpgrader->cancel();
}


void FirmwareUpgradeController::askForFirmwareFile(void)
{
    auto dialogTitle   = QStringLiteral("Select firmware file.");
    auto filesFormat   = QStringLiteral("Firmware Files (*.img)");
    auto firstLocation = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    auto firmwareFilename = QGCQFileDialog::
            getOpenFileName(nullptr, dialogTitle, firstLocation, filesFormat);

    emit infoMsgReceived("Selected file: " + firmwareFilename);

    flash(firmwareFilename);
}
