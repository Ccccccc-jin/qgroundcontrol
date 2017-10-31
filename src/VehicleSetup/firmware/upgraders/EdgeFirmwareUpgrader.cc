#include <QtRemoteObjects>
#include <memory>

#include "EdgeFirmwareUpgrader.h"
#include "rep_FirmwareUpgraderWatcher_replica.h"


class EdgeFirmwareUpgraderPrivate
{

public:
    enum State {
        Started = 0,
        Finished,
        UnexpectedError,
        DeviceFound,
        DeviceNotFound,
        OpenImageFailed,
        OpenDeviceFailed,
        ImageReadingFailed,
        DeviceWritingFailed
    };

    EdgeFirmwareUpgraderPrivate() {}
    EdgeFirmwareUpgraderPrivate(EdgeFirmwareUpgraderPrivate const& )      = delete;
    EdgeFirmwareUpgraderPrivate(EdgeFirmwareUpgraderPrivate&& )           = delete;
    EdgeFirmwareUpgraderPrivate& operator=(EdgeFirmwareUpgrader const&)   = delete;
    EdgeFirmwareUpgraderPrivate& operator=(EdgeFirmwareUpgrader&&)        = delete;

    QProcess fwUpgraderProcess;
    std::unique_ptr<FirmwareUpgraderWatcherReplica> watcher;
};


EdgeFirmwareUpgrader::EdgeFirmwareUpgrader(QObject* parent)
    : FirmwareUpgrader(parent),
      _pimpl(new EdgeFirmwareUpgraderPrivate())
{ }


EdgeFirmwareUpgrader::~EdgeFirmwareUpgrader(void)
{
    delete _pimpl;
}


void EdgeFirmwareUpgrader::flash(FirmwareImage const* fwImg)
{
    static auto const fwUpgraderBinary = QStringLiteral("");

    auto& upgraderProcess = _pimpl-> fwUpgraderProcess;

    upgraderProcess.start(fwUpgraderBinary, QStringList({fwImg->binFilename()}));
    QObject::connect(&upgraderProcess, &QProcess::errorOccurred,
                     this, &EdgeFirmwareUpgrader::_fwUpgraderProcessErrrorOcurred);

    QRemoteObjectNode clientNode;
    auto successful = clientNode.connectToNode(QUrl("local:fiw_upgrader"));

    if (!successful) {
        emit errorMessageReceived("Can not connect to firmware upgrader process");
        return;
    }
    emit statusMessageReceived("Successfully connected to firmware upgrader process");

    auto& watcher = _pimpl->watcher;

    watcher.reset(clientNode.acquire<FirmwareUpgraderWatcherReplica>());
    QObject::connect(watcher.get(), &FirmwareUpgraderWatcherReplica::initialized,
                     this, &EdgeFirmwareUpgrader::_onWatcherInitialized);
}


void EdgeFirmwareUpgrader::_onWatcherInitialized(void)
{
    auto& watcher = _pimpl->watcher;

    QObject::connect(watcher.get(), &FirmwareUpgraderWatcherReplica::subsystemStateChanged,
                     this, &EdgeFirmwareUpgrader::_onWatcherSubsystemStateChanged);

    QObject::connect(watcher.get()  ,&FirmwareUpgraderWatcherReplica::flasherProgressChanged,
                     this, &EdgeFirmwareUpgrader::flasherProgressChanged);

    watcher->start();

    emit statusMessageReceived("Firmware upgrader successfully initialized");
}


void EdgeFirmwareUpgrader::_fwUpgraderProcessErrrorOcurred(QProcess::ProcessError err)
{
    switch(err) {
        case QProcess::FailedToStart:
            emit errorMessageReceived("Failed to start firmware upgrader process");
            break;

        case QProcess::Crashed:
            emit errorMessageReceived("Firmware upgrader process crashed");
            break;

        case QProcess::UnknownError: default:
            emit errorMessageReceived("Unknown errror (Internal error)");
    }
}


void EdgeFirmwareUpgrader::_onWatcherSubsystemStateChanged(QString subsystem, uint state)
{

    if (state == EdgeFirmwareUpgraderPrivate::Finished
    &&  subsystem == QStringLiteral("flasher")) {
        emit flashingCompleted();
        return;
    }

    switch(state) {
        case EdgeFirmwareUpgraderPrivate::Started: {
            emit statusMessageReceived(subsystem + ": started.");
            break;
        }

        case EdgeFirmwareUpgraderPrivate::Finished: {
            emit statusMessageReceived(subsystem + ": finished.");
            break;
        }

        case EdgeFirmwareUpgraderPrivate::DeviceNotFound: {
            emit deviceNotFound();
            break;
        }

        case EdgeFirmwareUpgraderPrivate::DeviceFound: {
            emit statusMessageReceived(subsystem + ": device found.");
            break;
        }

        case EdgeFirmwareUpgraderPrivate::OpenImageFailed: {
            emit errorMessageReceived(subsystem + ": can not open image.");
            break;
        }

        case EdgeFirmwareUpgraderPrivate::OpenDeviceFailed: {
            emit errorMessageReceived(subsystem + ": can not open device.");
            break;
        }

        case EdgeFirmwareUpgraderPrivate::ImageReadingFailed: {
            emit errorMessageReceived(subsystem + ": image reading failed.");
            break;
        }

        case EdgeFirmwareUpgraderPrivate::DeviceWritingFailed: {
            emit errorMessageReceived(subsystem + ": writing to device failed.");
            break;
        }

        case EdgeFirmwareUpgraderPrivate::UnexpectedError: {
            emit errorMessageReceived(subsystem + ": unexpected error.");
            break;
        }

        default: {
            emit errorMessageReceived("Internal error");
        }
    }
}
