#include <QtRemoteObjects>
#include <memory>
#include <libusb-1.0/libusb.h>

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
        DeviceWritingFailed,
        CheckingCorrectness,
        ImageUncorrectlyWrote,
        ImageCorrectlyWrote
    };

    EdgeFirmwareUpgraderPrivate(EdgeFirmwareUpgraderPrivate const& )      = delete;
    EdgeFirmwareUpgraderPrivate(EdgeFirmwareUpgraderPrivate&& )           = delete;
    EdgeFirmwareUpgraderPrivate& operator=(EdgeFirmwareUpgrader const&)   = delete;
    EdgeFirmwareUpgraderPrivate& operator=(EdgeFirmwareUpgrader&&)        = delete;

    EdgeFirmwareUpgraderPrivate()
        : checksumEnabled(true),
          image(nullptr)
    {  }

    bool checksumEnabled;
    QProcess fwUpgraderProcess;
    QRemoteObjectNode node;
    FirmwareImage* image;
    FirmwareUpgraderWatcherReplica* watcher;
};


EdgeFirmwareUpgrader::EdgeFirmwareUpgrader(QObject* parent)
    : FirmwareUpgrader(parent),
      _pimpl(new EdgeFirmwareUpgraderPrivate())
{ }


EdgeFirmwareUpgrader::~EdgeFirmwareUpgrader(void)
{
    _pimpl->fwUpgraderProcess.kill();
    _pimpl->fwUpgraderProcess.waitForFinished();

    delete _pimpl;
}


bool EdgeFirmwareUpgrader::deviceAvailable() const
{
    auto const requiredVid = 0x0a5c;
    auto requiredPids = QList<int>({ 0x2764, 0x2763 });

    libusb_context *context = 0;
    libusb_device **list = 0;
    int ret = 0;
    ssize_t count = 0;
    int bootable = 0;

    ret = libusb_init(&context);
    Q_ASSERT(ret == 0);

    count = libusb_get_device_list(context, &list);
    Q_ASSERT(count > 0);

    for (ssize_t idx = 0; idx < count; ++idx) {
        libusb_device *device = list[idx];
        struct libusb_device_descriptor desc;

        ret = libusb_get_device_descriptor(device, &desc);
        Q_ASSERT(ret == 0);

        if (desc.idVendor == requiredVid && requiredPids.contains(desc.idProduct)) {
           bootable++;
        }
    }

    libusb_exit(context);
    return bootable > 0;
}


void EdgeFirmwareUpgrader::cancel(void)
{
    _pimpl->watcher->cancel();
}


void EdgeFirmwareUpgrader::enableChecksum(bool checksumEnabled)
{
    _pimpl-> checksumEnabled = checksumEnabled;
}


bool EdgeFirmwareUpgrader::checksumEnabled(void) const
{
    return _pimpl-> checksumEnabled;
}


void EdgeFirmwareUpgrader::reboot(void)
{

}


FirmwareImage* EdgeFirmwareUpgrader::image(void) const
{
    return _pimpl->image;
}


void EdgeFirmwareUpgrader::flash(FirmwareImage* fwImg)
{
    _pimpl->image = fwImg;
    static auto const fwUpgraderBinary = QStringLiteral("/home/vladimir.provalov/dev/qgcflasher/build-firmwareupgrader-fwupgrader-Debug/fwupgrader");


    _pimpl->fwUpgraderProcess.start("gksudo " + fwUpgraderBinary);
    qInfo() << fwImg->binFilename();

    QObject::connect(&_pimpl->fwUpgraderProcess, &QProcess::errorOccurred,
                     this, &EdgeFirmwareUpgrader::_fwUpgraderProcessErrrorOcurred);

    QObject::connect(&_pimpl->fwUpgraderProcess, &QProcess::stateChanged,
                     this, &EdgeFirmwareUpgrader::_onProcessStateChanged);

    _pimpl->fwUpgraderProcess.waitForStarted();

    auto successful = _pimpl->node.connectToNode(QUrl("local:fwupg_socket"));

    if (!successful) {
        emit errorMessageReceived("Can not connect to firmware upgrader process");
        return;
    }

    _pimpl->watcher = _pimpl->node.acquire<FirmwareUpgraderWatcherReplica>();
    QObject::connect(_pimpl->watcher, &FirmwareUpgraderWatcherReplica::initialized,
         this, &EdgeFirmwareUpgrader::_onWatcherInitialized);

}


void EdgeFirmwareUpgrader::_onWatcherInitialized(void)
{
    emit statusMessageReceived("Firmware upgrader successfully initialized");

    QObject::connect(_pimpl->watcher, &FirmwareUpgraderWatcherReplica::subsystemStateChanged,
                     this, &EdgeFirmwareUpgrader::_onWatcherSubsystemStateChanged);

    QObject::connect(_pimpl->watcher  ,&FirmwareUpgraderWatcherReplica::flasherProgressChanged,
                     this, &EdgeFirmwareUpgrader::flasherProgressChanged);

    QObject::connect(_pimpl-> watcher, &FirmwareUpgraderWatcherReplica::finished,
         [this] () {
             emit statusMessageReceived("Firmware upgrading completed.");
         }
    );

    QObject::connect(this, &EdgeFirmwareUpgrader::start,
                     _pimpl->watcher, &FirmwareUpgraderWatcherReplica::start);

    emit start(_pimpl->image->binFilename(), _pimpl->checksumEnabled);
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


void EdgeFirmwareUpgrader::_onProcessStateChanged(QProcess::ProcessState state)
{
    switch(state) {
        case QProcess::NotRunning:
            emit statusMessageReceived("Firmware upgrader not running.");
            break;

        case QProcess::Starting:
            emit statusMessageReceived("Firmware upgrader starting.");
            break;

        case QProcess::Running:
            emit statusMessageReceived("Firmware upgrader running");
            break;
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
            emit statusMessageReceived(subsystem + " : started...");
            break;
        }

        case EdgeFirmwareUpgraderPrivate::Finished: {
            emit statusMessageReceived(subsystem + " : succesfully finished.");
            break;
        }

        case EdgeFirmwareUpgraderPrivate::DeviceNotFound: {
            emit errorMessageReceived(subsystem + " : device not found.");
            emit deviceNotFound();
            break;
        }

        case EdgeFirmwareUpgraderPrivate::DeviceFound: {
            emit statusMessageReceived(subsystem + " : device found.");
            break;
        }

        case EdgeFirmwareUpgraderPrivate::OpenImageFailed: {
            emit errorMessageReceived(subsystem + " : can not open image.");
            break;
        }

        case EdgeFirmwareUpgraderPrivate::OpenDeviceFailed: {
            emit errorMessageReceived(subsystem + " : can not open device.");
            break;
        }

        case EdgeFirmwareUpgraderPrivate::ImageReadingFailed: {
            emit errorMessageReceived(subsystem + " : image reading failed.");
            break;
        }

        case EdgeFirmwareUpgraderPrivate::DeviceWritingFailed: {
            emit errorMessageReceived(subsystem + " : device writing failed.");
            break;
        }

        case EdgeFirmwareUpgraderPrivate::UnexpectedError: {
            emit errorMessageReceived(subsystem + " : unexpected error.");
            break;
        }

        case EdgeFirmwareUpgraderPrivate::CheckingCorrectness: {
            emit statusMessageReceived(subsystem + " : checking correctness of flashing...");
            break;
        }

        case EdgeFirmwareUpgraderPrivate::ImageUncorrectlyWrote: {
            emit errorMessageReceived(subsystem + " : image uncorrectly wrote.");
            break;
        }

        case EdgeFirmwareUpgraderPrivate::ImageCorrectlyWrote: {
            emit statusMessageReceived(subsystem + " : image correctly wrote.");
            break;
        }

        default: {
            emit errorMessageReceived("Internal error");
        }
    }
}
