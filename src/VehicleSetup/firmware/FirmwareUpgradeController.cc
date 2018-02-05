#include <QStandardPaths>
#include <QDebug>

#include "FirmwareUpgradeController.h"
#include "FirmwareUpgraderInterface.h"
#include "QGCQFileDialog.h"


FirmwareUpgradeController::FirmwareUpgradeController(void)
    : _firmwareDirectory(""),
      _firmwareFilename(""),
      _checksumEnabled(true),
      _firmwareSavingEnabled(false),
      _updateMethod(UpdateMethod::Auto),
      _deviceObserver(1000),
      _remoteFirmwareInfoView(new RemoteFirmwareInfoView()),
      _fwUpgrader(std::move(FirmwareUpgrader::instance()))
{
    FirmwareUpgrader::registerMetatypes();
    _initConnections();
}


FirmwareUpgradeController::~FirmwareUpgradeController(void)
{
    qInfo() << "FirmwareUpgradeController destructed.";
}


void FirmwareUpgradeController::initializeDevice(void)
{
    _fwUpgrader->initializeDevice();
}


bool FirmwareUpgradeController::_deviceAvailable(void)
{
    return _fwUpgrader->deviceAvailable();
}


void FirmwareUpgradeController::_initConnections(void)
{
    return _checksumEnabled;
}


void FirmwareUpgradeController::enableChecksum(bool checksumEnabled)
{
    _checksumEnabled = checksumEnabled;
}


void FirmwareUpgradeController::_attachFirmwareUpgrader(void)
{
    using FWUpgrader = FirmwareUpgrader;
    using Controller = FirmwareUpgradeController;

    auto fwUpgraderPtr = _fwUpgrader.get();

    QObject::connect(this, &Controller::_cancel, fwUpgraderPtr, &FWUpgrader::cancel);

    QObject::connect(fwUpgraderPtr, &FWUpgrader::infoMessageReceived,         this, &Controller::infoMsgReceived);
    QObject::connect(fwUpgraderPtr, &FWUpgrader::errorMessageReceived,        this, &Controller::errorMsgReceived);
    QObject::connect(fwUpgraderPtr, &FWUpgrader::warnMessageReceived,         this, &Controller::warnMsgReceived);
    QObject::connect(fwUpgraderPtr, &FWUpgrader::progressChanged,             this, &Controller::flasherProgressChanged);
    QObject::connect(fwUpgraderPtr, &FWUpgrader::deviceInitialized,           this, &Controller::deviceInitialized);
    QObject::connect(fwUpgraderPtr, &FWUpgrader::cancelled,                   this, &Controller::_onCancelled);
    QObject::connect(fwUpgraderPtr, &FWUpgrader::deviceFlashed,               this, &Controller::deviceFlashed);
    QObject::connect(fwUpgraderPtr, &FWUpgrader::deviceInitializationStarted, this, &Controller::deviceInitializationStarted);

    QObject::connect(fwUpgraderPtr, &FWUpgrader::deviceInitialized,
        [this] (bool status) {
            if (status) {
                auto remoteFwVersion = _remoteFirmwareInfoView->remoteFirmwareInfo().version();

                auto msg = QString("");

    QObject::connect(this, &Controller::deviceInitializationStarted,
                     [this] () { _deviceObserver.stop(); } );

    QObject::connect(fwUpgraderPtr, &FWUpgrader::firmwareVersionAvailable,
        [this] (QString const& firmwareVersion) {
            _firmwareVersion = FirmwareVersion::fromString(firmwareVersion);
            emit firmwareVersionAvailable(firmwareVersion);
        }
    );
}


void FirmwareUpgradeController::observeDevice(void)
{
    qInfo() << "Polling started...";
    _deviceObserver.setDeviceAvailablePredicate(
        [this] () { return _fwUpgrader->deviceAvailable(); }
    );

    _deviceObserver.observe();
}


void FirmwareUpgradeController::flash(void)
{
    emit deviceFlashingStarted();
    if (_firmwareFilename.isEmpty()) {
        emit warnMsgReceived("File is not selected.");
        return;
    }
    FlasherParameters params(FirmwareImage(_firmwareFilename), _checksumEnabled);
    _fwUpgrader->flash(params);
}


void FirmwareUpgradeController::cancel(void)
{
    _fwUpgrader->cancel();
}


void FirmwareUpgradeController::askForFirmwareFile(void)
{
    auto dialogTitle   = QStringLiteral("Select firmware file.");
    auto filesFormat   = QStringLiteral("Firmware Files (*.img)");
    auto firstLocation = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    auto fwFilename = QGCQFileDialog::
            getOpenFileName(nullptr, dialogTitle, firstLocation, filesFormat);

    if (fwFilename.isEmpty()) {
        if (_firmwareFilename.isEmpty()) {
            emit warnMsgReceived("File is not selected.");
        }
    } else {
        _firmwareFilename = std::move(fwFilename);
        emit infoMsgReceived("Selected file: " + _firmwareFilename);
    }
}


void FirmwareUpgradeController::askForFirmwareDirectory(void)
{
    auto dialogTitle   = QStringLiteral("Select directory");
    auto firstLocation = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    auto firmwareDirectory = QGCQFileDialog::
            getExistingDirectory(nullptr, dialogTitle, firstLocation);

    if (firmwareDirectory.isEmpty()) {
        if (_firmwareDirectory.isEmpty()) {
            emit warnMsgReceived("Directory not selected.");
        }
    } else {
        _firmwareDirectory = std::move(firmwareDirectory);
        emit infoMsgReceived("Selected directory: " + _firmwareDirectory);
        _remoteFwManager.setDestDirPath(_firmwareDirectory);
        _availableDiskSpace = QStorageInfo(_firmwareDirectory).bytesFree();
        emit availableDiskSpaceChanged();
    }
}


bool FirmwareUpgradeController::hasEnoughDiskSpace(void)
{
    auto info = _remoteFirmwareInfoView->remoteFirmwareInfo();
    return (info.imageSize() + info.archiveSize()) < _availableDiskSpace
            || _remoteFwManager.cached();
}
