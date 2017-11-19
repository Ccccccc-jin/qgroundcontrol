#include <QStandardPaths>
#include <QDebug>

#include "FirmwareUpgradeController.h"
#include "FirmwareUpgraderInterface.h"
#include "QGCQFileDialog.h"


FirmwareUpgradeController::FirmwareUpgradeController(void)
    : _firmwareFilename(""),
      _firmwareVersion(""),
      _checksumEnabled(true),
      _fwUpgrader(std::move(FirmwareUpgrader::instance()))
{
    _connectToFirmwareUpgrader();
}


FirmwareUpgradeController::~FirmwareUpgradeController(void)
{
    qInfo() << "FirmwareUpgradeController destructed.";
}


void FirmwareUpgradeController::start(void)
{
    _fwUpgrader->start();
}


bool FirmwareUpgradeController::deviceAvailable(void) const
{
    return _fwUpgrader->deviceAvailable();
}


bool FirmwareUpgradeController::checksumEnabled(void) const
{
    return _checksumEnabled;
}


void FirmwareUpgradeController::enableChecksum(bool checksumEnabled)
{
    _checksumEnabled = checksumEnabled;
}


void FirmwareUpgradeController::_connectToFirmwareUpgrader(void) {
    auto fwUpgraderPtr = _fwUpgrader.get();

    QObject::connect(fwUpgraderPtr, &FirmwareUpgrader::infoMessageReceived,
                     this,          &FirmwareUpgradeController::infoMsgReceived);

    QObject::connect(fwUpgraderPtr, &FirmwareUpgrader::errorMessageReceived,
                     this,          &FirmwareUpgradeController::errorMsgReceived);

    QObject::connect(fwUpgraderPtr, &FirmwareUpgrader::warnMessageReceived,
                     this,          &FirmwareUpgradeController::warnMsgReceived);

    QObject::connect(fwUpgraderPtr, &FirmwareUpgrader::progressChanged,
                     this,          &FirmwareUpgradeController::flasherProgressChanged);

    QObject::connect(fwUpgraderPtr, &FirmwareUpgrader::flashingFinished,
                     this,          &FirmwareUpgradeController::flashingFinished);

    QObject::connect(fwUpgraderPtr, &FirmwareUpgrader::finished,
                     this,          &FirmwareUpgradeController::finished);

    QObject::connect(fwUpgraderPtr, &FirmwareUpgrader::firmwareVersionAvailable,
        [this] (QString const& firmwareVersion) {
            _firmwareVersion = firmwareVersion;
            emit firmwareVersionAvailable(firmwareVersion);

        }
    );

    QObject::connect(fwUpgraderPtr, &FirmwareUpgrader::started,
                     this,          &FirmwareUpgradeController::started);

    QObject::connect(fwUpgraderPtr, &FirmwareUpgrader::cancelled,
                     this,          &FirmwareUpgradeController::cancelled);

    QObject::connect(fwUpgraderPtr, &FirmwareUpgrader::finished,
                     this,          &FirmwareUpgradeController::finished);
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


void FirmwareUpgradeController::searchDevice(void)
{
    qInfo() << "Polling started...";
    _startPolling();
}


void FirmwareUpgradeController::flash(void)
{
    FlasherParameters params(FirmwareImage(_firmwareFilename), _checksumEnabled);
    _fwUpgrader->flash(params);
}


void FirmwareUpgradeController::cancel(void)
{
    _fwUpgrader->finish();
}


void FirmwareUpgradeController::askForFirmwareFile(void)
{
    auto dialogTitle   = QStringLiteral("Select firmware file.");
    auto filesFormat   = QStringLiteral("Firmware Files (*.img)");
    auto firstLocation = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    _firmwareFilename = QGCQFileDialog::
            getOpenFileName(nullptr, dialogTitle, firstLocation, filesFormat);

    emit infoMsgReceived("Selected file: " + _firmwareFilename);
}
