#include <QStandardPaths>
#include <QDebug>

#include "FirmwareUpgradeController.h"
#include "FirmwareUpgraderInterface.h"
#include "QGCQFileDialog.h"


FirmwareUpgradeController::FirmwareUpgradeController(void)
    : _firmwareFilename(""),
      _firmwareVersion(""),
      _checksumEnabled(true),
      _deviceBootAsMassStorage(false),
      _deviceObserver(1),
      _fwUpgrader(std::move(FirmwareUpgrader::instance()))
{
    _initConnections();
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


void FirmwareUpgradeController::_initConnections(void)
{
    using FWUpgrader = FirmwareUpgrader;
    using Controller = FirmwareUpgradeController;

    auto fwUpgraderPtr = _fwUpgrader.get();

    QObject::connect(fwUpgraderPtr, &FWUpgrader::infoMessageReceived,  this, &Controller::infoMsgReceived);
    QObject::connect(fwUpgraderPtr, &FWUpgrader::errorMessageReceived, this, &Controller::errorMsgReceived);
    QObject::connect(fwUpgraderPtr, &FWUpgrader::warnMessageReceived,  this, &Controller::warnMsgReceived);
    QObject::connect(fwUpgraderPtr, &FWUpgrader::progressChanged,      this, &Controller::flasherProgressChanged);
    QObject::connect(fwUpgraderPtr, &FWUpgrader::flashingFinished,     this, &Controller::flashingFinished);
    QObject::connect(fwUpgraderPtr, &FWUpgrader::ready,                this, &Controller::ready);
    QObject::connect(fwUpgraderPtr, &FWUpgrader::cancelled,            this, &Controller::cancelled);
    QObject::connect(fwUpgraderPtr, &FWUpgrader::finished,             this, &Controller::finished);
    QObject::connect(fwUpgraderPtr, &FWUpgrader::initialzed,          [this] () { _deviceObserver.stop(); } );

    QObject::connect(fwUpgraderPtr, &FWUpgrader::firmwareVersionAvailable,
        [this] (QString const& firmwareVersion) {
            _firmwareVersion = firmwareVersion;
            emit firmwareVersionAvailable(firmwareVersion);
        }
    );

    QObject::connect(&_deviceObserver, &DeviceObserver::devicePlugged,   this, &Controller::devicePlugged);
    QObject::connect(&_deviceObserver, &DeviceObserver::deviceUnplugged, this, &Controller::deviceUnplugged);

}


void FirmwareUpgradeController::searchDevice(void)
{
    qInfo() << "Polling started...";
    _deviceObserver.setDeviceAvailablePredicate(
        [this] () { return _fwUpgrader->deviceAvailable(); }
    );

    _deviceObserver.observe();
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
