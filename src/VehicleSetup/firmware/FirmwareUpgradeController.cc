#include <QStandardPaths>
#include <QDebug>

#include "FirmwareUpgradeController.h"
#include "FirmwareUpgraderInterface.h"
#include "QGCQFileDialog.h"
#include "QGCFileDownload.h"
#include "QGCXzDecompressor.h"
#include "QGCApplication.h"


FirmwareUpgradeController::FirmwareUpgradeController(void)
    : _firmwareFilename(""),
      _firmwareSavingEnabled(false),
      _updateMethod(UpdateMethod::Auto),
      _deviceObserver(1000),
      _remoteFirmwareInfoView(new RemoteFirmwareInfoView()),
      _fwUpgrader(FirmwareUpgrader::instance())
{
    FirmwareUpgrader::registerMetatypes();

    auto destPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    if (destPath.isEmpty()) {
        destPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
        if (destPath.isEmpty()) {
            qWarning() << "Can not get destPath for firmwareManager";
        }
    }

    _remoteFwManager.setDestDirPath(destPath);
    _initConnections();
}


void FirmwareUpgradeController::_initConnections(void)
{
    _attachFirmwareUpgrader();
    _attachDeviceObserver();
    _attachFirmwareManager();

    using Controller = FirmwareUpgradeController;
    QObject::connect(this, &Controller::_flash, this, &Controller::_flashSelectedFile);
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

                if (!remoteFwVersion.defined()) {
                    msg = "Can not get info about remote firmware. Check for internet connection."
                          " You can try to flash Edge with manually selected firmware in 'Advanced settings'.";

                } else  if (remoteFwVersion > _firmwareVersion) {
                    msg = QString("New firmware available. Date ") + remoteFwVersion.releaseDate()
                                + "\nClick 'Ok' and QGround automatically download firmware and flash Edge.";

                } else if (remoteFwVersion == _firmwareVersion) {
                    msg = "Your firmware is up to date. Cancel upgrading or select firmware manually in 'Advanced settings'";
                }

                emit firmwareInfoMsg(msg) ;

                _availableDiskSpace = QStorageInfo(_remoteFwManager.destDirPath()).bytesFree();
                emit availableDiskSpaceChanged();
            }
        }
    );

    QObject::connect(fwUpgraderPtr, &FWUpgrader::firmwareVersionAvailable,
        [this] (QString const& firmwareVersion) {
            _firmwareVersion = FirmwareVersion::fromString(firmwareVersion);
            emit firmwareVersionAvailable(firmwareVersion);
        }
    );
}


void FirmwareUpgradeController::_attachDeviceObserver(void)
{
    using Controller = FirmwareUpgradeController;

    QObject::connect(&_deviceObserver, &DeviceObserver::devicePlugged,   this, &Controller::devicePlugged);
    QObject::connect(&_deviceObserver, &DeviceObserver::deviceUnplugged, this, &Controller::deviceUnplugged);
    QObject::connect(this, &Controller::deviceInitializationStarted,    [this] () { _deviceObserver.stop(); } );
}


void FirmwareUpgradeController::_fetchFirmwareInfo(void)
{
    auto dataBuffer      = std::shared_ptr<QByteArray>(new QByteArray());
    auto downloadWatcher = QGCDownload::
            download(EdgeRemoteFirmwareInfo::firmwareInfoFileUrl(), dataBuffer);

    QObject::connect(downloadWatcher.get(), &QGCDownloadWatcher::success,
        [this, dataBuffer] (void) {
            auto latestFwInfo = EdgeRemoteFirmwareInfo();
            latestFwInfo.fromJson(*dataBuffer);

            auto cashedFwVersion = _remoteFirmwareInfoView->remoteFirmwareInfo().version();

            if (cashedFwVersion != latestFwInfo.version()) {
                _remoteFwManager.resetCache();
                _remoteFwManager.setSourceUrl(QString(EdgeRemoteFirmwareInfo::firmwareUrl())
                                              + "/" + latestFwInfo.firmwareArchiveName());
                _remoteFirmwareInfoView->setRemoteFirmwareInfo(latestFwInfo);

                emit remoteFirmwareInfoChanged();
            }
        }
    );

    QObject::connect(downloadWatcher.get(), &QGCDownloadWatcher::networkError,
        [this] (QNetworkReply::NetworkError error) {
            Q_UNUSED(error);
            auto undefinedFirmwareInfo = EdgeRemoteFirmwareInfo();
            _remoteFirmwareInfoView->setRemoteFirmwareInfo(undefinedFirmwareInfo);
            emit remoteFirmwareInfoChanged();
        }
    );

    _downloadWatcher = std::move(downloadWatcher);
}


void FirmwareUpgradeController::_attachFirmwareManager(void)
{
    using Controller = FirmwareUpgradeController;

    QObject::connect(&_remoteFwManager, &RemoteFirmwareManager::networkError,
                     this,              &Controller::_onNetworkError);

    QObject::connect(&_remoteFwManager, &RemoteFirmwareManager::decompressError,
                     this,              &Controller::_onDecompressError);

    QObject::connect(&_remoteFwManager, &RemoteFirmwareManager::cancelled,
                     this,              &Controller::_onCancelled);

    QObject::connect(&_remoteFwManager, &RemoteFirmwareManager::savingError,
        [this] (void) {
            emit errorMsgReceived("Can not save file. Not enough disk space. "
                                  "You can select another location in 'Advanced settings'");
            emit deviceFlashed(false);
        }
    );

    QObject::connect(&_remoteFwManager, &RemoteFirmwareManager::firmwareExtracted,
        [this] (QString filename) {
            emit infoMsgReceived("Firmware extracted. Flashing...");
            _firmwareFilename = filename;
            _flashSelectedFile();
        }
    );

    QObject::connect(&_remoteFwManager, &RemoteFirmwareManager::firmwareDownloaded,
        [this] (QString localFile)
            { Q_UNUSED(localFile); emit infoMsgReceived("Firmware downloaded. Extracting..."); }
    );

    QObject::connect(this,              &Controller::_cancel,
                     &_remoteFwManager, &RemoteFirmwareManager::cancel);

    QObject::connect(&_remoteFwManager, &RemoteFirmwareManager::progressChanged,
        [this] (qint64 curr, qint64 total) { emit flasherProgressChanged((curr * 100) / total); }
    );

    QObject::connect(&_remoteFwManager, &RemoteFirmwareManager::archiveCached,
        [this] (void) { emit infoMsgReceived("Skip downloading. Archive cached. Extracting..."); }
    );

    QObject::connect(&_remoteFwManager, &RemoteFirmwareManager::firmwareCached,
        [this] (void) {
            emit infoMsgReceived("Skip extracting. Firmware cached.");
            _flashSelectedFile();
        }
    );
}


void FirmwareUpgradeController::_removeDownloadedFiles(void)
{
    auto remoteFwInfo = _remoteFirmwareInfoView->remoteFirmwareInfo();
    if (remoteFwInfo.isUndefined()) {
        return;
    }

    auto fwArchiveName    = remoteFwInfo.firmwareArchiveName();
    auto filesLocation    = QDir(_remoteFwManager.destDirPath());

    auto archiveLocalPath = QFileInfo(filesLocation, fwArchiveName).absoluteFilePath();
    auto imageLocalPath   = QGCXzDecompressor::eraseXzSuffix(archiveLocalPath);

    auto removeFile = [] (QString const& fileName) {
        auto fileInfo = QFileInfo(fileName);
        if (fileInfo.isFile() && fileInfo.exists()) {
            if (!QFile::remove(fileName)) {
                qWarning() << "Can not remove downloaded file: " << fileName;
            }
        }
    };

    removeFile(archiveLocalPath);
    removeFile(imageLocalPath);
}


FirmwareUpgradeController::~FirmwareUpgradeController(void)
{
    if (_updateMethod == UpdateMethod::Auto && !_firmwareSavingEnabled) {
        _removeDownloadedFiles();
    }
}


void FirmwareUpgradeController::initializeDevice(void)
{
    _fwUpgrader->initializeDevice();
    _fetchFirmwareInfo();
}


void FirmwareUpgradeController::_flashSelectedFile(void)
{
    if (_firmwareFilename.isEmpty()) {
        emit warnMsgReceived("File is not selected.");
        return;
    }

    FlasherParameters params(FirmwareImage(_firmwareFilename),
                             _settings.checksumEnabeld());
    _fwUpgrader->flash(params);
}


bool FirmwareUpgradeController::_deviceAvailable(void)
{
    return _fwUpgrader->deviceAvailable();
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
    emit firmwareUpgraderStarted();

    if (_updateMethod == UpdateMethod::Manual) {
        _flashSelectedFile();
    } else {
        auto fwInfo = _remoteFirmwareInfoView->remoteFirmwareInfo();
        auto neededDiskSpace = fwInfo.imageSize() + fwInfo.archiveSize();

        if (neededDiskSpace > _availableDiskSpace) {
            emit errorMsgReceived("Not enough disk space.");
            emit deviceFlashed(false);
            return;
        }

        emit infoMsgReceived("Downloading firmware...");
        if (!_remoteFwManager.asyncRun()) {
            emit errorMsgReceived("Can not start downloading."
                                  " Check for internet connection");
            emit deviceFlashed(false);
        }
    }
}


void FirmwareUpgradeController::cancel(void)
{
    emit _cancel();
}


void FirmwareUpgradeController::_onNetworkError(QNetworkReply::NetworkError error)
{
    using NetError = QNetworkReply::NetworkError;

    if (error == NetError::OperationCanceledError) {
        return;
    }

    switch(error) {
        case NetError::ContentNotFoundError: {
            emit errorMsgReceived("Can not download remote file. File not found.");
            break;
        }

        case NetError::TimeoutError: {
            emit errorMsgReceived("Can not download remote file. Connection timed out.");
            break;
        }

        default: {
            emit errorMsgReceived(QString("File downloading failed. Code: ") + QString::number(error));
        }
    }

    emit deviceFlashed(false);
}


void FirmwareUpgradeController::_onDecompressError(QGCXzDecompressor::ErrorType error)
{
    using DecompressError = QGCXzDecompressor::ErrorType;

    switch(error) {
        case DecompressError::OpenArchiveFailed: {
            emit errorMsgReceived("Can not open archive file.");
            break;
        }

        case DecompressError::OpenDestFailed: {
            emit errorMsgReceived("Can not open destination file.");
            break;
        }

        case DecompressError::CorruptData: {
            emit errorMsgReceived("Can not extract firmware. Archive is corrupt.");
            break;
        }

        case DecompressError::MagicNumberError: {
            emit errorMsgReceived("This archive file not supported.");
            break;
        }

        case DecompressError::WriteDestFailed: {
            emit errorMsgReceived("Can not write to file. Make sure that you have enough space");
            break;
        }

        default: {
            emit errorMsgReceived(QString("Extracting failed, code: %1").arg(error));
        }
    }
    emit deviceFlashed(false);
}


void FirmwareUpgradeController::_onCancelled(void)
{
    emit cancelled();
}


void FirmwareUpgradeController::askForFirmwareFile(void)
{
    auto dialogTitle   = QStringLiteral("Select firmware file.");
    auto filesFormat   = QStringLiteral("Firmware Files (*.img)");
    auto firstLocation = _settings.defaultFirmwareSearchPath();

    auto fwFilename = QGCQFileDialog::
            getOpenFileName(nullptr, dialogTitle, firstLocation, filesFormat);

    if (fwFilename.isEmpty()) {
        if (_firmwareFilename.isEmpty()) {
            emit warnMsgReceived("File is not selected.");
        }
    } else {
        _firmwareFilename = std::move(fwFilename);
        emit infoMsgReceived("Selected file: " + _firmwareFilename);

        auto firmwareDir = QFileInfo(_firmwareFilename).dir().path();
        _settings.setDefaultFirmwareSearchPath(firmwareDir);
    }
}


void FirmwareUpgradeController::askForFirmwareDirectory(void)
{
    auto dialogTitle   = QStringLiteral("Select directory");
    auto const& firstLocation = _settings.defaultFirmwareSavePath();

    auto firmwareDirectory = QGCQFileDialog::
            getExistingDirectory(nullptr, dialogTitle, firstLocation);

    if (firmwareDirectory.isEmpty()) {
        emit infoMsgReceived("Fimrware will be saved to: " + firstLocation);
    } else {
        _settings.setDefaultFirmwareSavePath(firmwareDirectory);
        emit infoMsgReceived("Selected directory: " + firmwareDirectory);

        _remoteFwManager.setDestDirPath(firmwareDirectory);
        _availableDiskSpace = QStorageInfo(firmwareDirectory).bytesFree();

        emit availableDiskSpaceChanged();
    }
}


bool FirmwareUpgradeController::hasEnoughDiskSpace(void)
{
    auto info = _remoteFirmwareInfoView->remoteFirmwareInfo();
    return (info.imageSize() + info.archiveSize()) < _availableDiskSpace
            || _remoteFwManager.cached();
}
