#include <QtRemoteObjects>
#include <memory>
#include <libusb-1.0/libusb.h>

#include "States.h"
#include "EdgeFirmwareUpgrader.h"
#include "rep_FirmwareUpgraderWatcher_replica.h"


class EdgeFirmwareUpgraderPrivate
{

public:
    explicit EdgeFirmwareUpgraderPrivate()
        : fwUpgraderParams(),
          upgraderWatcher()
    {  }

    static QString FW_UPGRADER_BINARY;
    static QUrl    FW_UPGRADER_NODE_URL;
    static QString GRAPHICAL_SUDO_COMMAND;

    static int        EDGE_VID;
    static QList<int> EDGE_PIDS;

    struct FwUpgraderParams {
        explicit FwUpgraderParams(void)
            : checksumEnabled(true),
              fwImage("", "")
        {  }

        bool checksumEnabled;
        FirmwareImage fwImage;
    };

    struct RemoteObjectData {
        explicit RemoteObjectData(void)
        {  }

        QProcess process;
        QRemoteObjectNode serverNode;
        FirmwareUpgraderWatcherReplica* replica;
    };

    FwUpgraderParams fwUpgraderParams;
    RemoteObjectData upgraderWatcher;
};

QString EdgeFirmwareUpgraderPrivate::FW_UPGRADER_BINARY =
        "/home/vladimir.provalov/dev/qgcflasher/build-firmwareupgrader-fwupgrader-Debug/fwupgrader";
QUrl    EdgeFirmwareUpgraderPrivate::FW_UPGRADER_NODE_URL = QUrl("local:fwupg_socket");
QString EdgeFirmwareUpgraderPrivate::GRAPHICAL_SUDO_COMMAND = "gksudo ";

int EdgeFirmwareUpgraderPrivate::EDGE_VID = 0x0a5c;
QList<int> EdgeFirmwareUpgraderPrivate::EDGE_PIDS = QList<int>({0x2763, 0x2764});


EdgeFirmwareUpgrader::EdgeFirmwareUpgrader(QObject* parent)
    : FirmwareUpgrader(parent),
      _messageHandler(this),
      _pimpl(new EdgeFirmwareUpgraderPrivate())
{ }


EdgeFirmwareUpgrader::~EdgeFirmwareUpgrader(void)
{
    _pimpl->upgraderWatcher.process.kill();
    _pimpl->upgraderWatcher.process.waitForFinished();

    delete _pimpl;
}


void EdgeFirmwareUpgrader::start(void)
{
    _runProcess();
    _connectToProcess();
}


EdgeFirmwareUpgrader::FirmwareInfo
    EdgeFirmwareUpgrader::firmwareInfo(void) const
{
    return EdgeFirmwareUpgrader::FirmwareInfo(
        123, EdgeFirmwareUpgrader::FirmwareInfo::Type::EDGE
    );
}


void EdgeFirmwareUpgrader::flash(FirmwareImage const& image)
{
    connect(_pimpl->upgraderWatcher.replica, &FirmwareUpgraderWatcherReplica::deviceScannerFinished,
        [this, image] { emit _flash(image.filename(), _pimpl->fwUpgraderParams.checksumEnabled); }
    );

    emit _runDeviceScanner();
}


bool EdgeFirmwareUpgrader::deviceAvailable() const
{
    auto const requiredVid  = EdgeFirmwareUpgraderPrivate::EDGE_VID;
    auto const requiredPids = EdgeFirmwareUpgraderPrivate::EDGE_PIDS;

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


void EdgeFirmwareUpgrader::_connectToProcessSignals(void)
{
    qDebug() << "Watcher initialized";

    QObject::connect(_pimpl->upgraderWatcher.replica, &FirmwareUpgraderWatcherReplica::rpiBootStateChanged,
                     &_messageHandler, &EdgeMessageHandler::onRpiBootStateChanged);

    QObject::connect(_pimpl->upgraderWatcher.replica, &FirmwareUpgraderWatcherReplica::flasherStateChanged,
                     &_messageHandler, &EdgeMessageHandler::onFlasherStateChanged);

    QObject::connect(_pimpl->upgraderWatcher.replica, &FirmwareUpgraderWatcherReplica::deviceScannerStateChanged,
                     &_messageHandler, &EdgeMessageHandler::onDeviceScannerStateChanged);

    QObject::connect(&_messageHandler, &EdgeMessageHandler::infoMessageReceived,
                     this, &EdgeFirmwareUpgrader::infoMessageReceived);

    QObject::connect(&_messageHandler, &EdgeMessageHandler::errorMessageReceived,
                     this, &EdgeFirmwareUpgrader::errorMessageReceived);

    QObject::connect(&_messageHandler, &EdgeMessageHandler::warnMessageReceived,
                     this, &EdgeFirmwareUpgrader::warnMessageReceived);

    QObject::connect(_pimpl->upgraderWatcher.replica, &FirmwareUpgraderWatcherReplica::flasherProgressChanged,
                     this, &EdgeFirmwareUpgrader::firmwareUpgraderProgressChanged);

    QObject::connect(this, &EdgeFirmwareUpgrader::_flash,
                     _pimpl->upgraderWatcher.replica, &FirmwareUpgraderWatcherReplica::runFlasherStep);

    QObject::connect(this, &EdgeFirmwareUpgrader::_runRpiboot,
                     _pimpl->upgraderWatcher.replica, &FirmwareUpgraderWatcherReplica::runRpiBootStep);

    QObject::connect(this, &EdgeFirmwareUpgrader::_runDeviceScanner,
                     _pimpl->upgraderWatcher.replica, &FirmwareUpgraderWatcherReplica::runDeviceScannerStep);

    QObject::connect(this, &EdgeFirmwareUpgrader::_setFilterParams,
                     _pimpl->upgraderWatcher.replica, &FirmwareUpgraderWatcherReplica::setFilterParams);

    QObject::connect(_pimpl->upgraderWatcher.replica, &FirmwareUpgraderWatcherReplica::rpiBootFinished,
                     this, &EdgeFirmwareUpgrader::_onRpiBootFinished);

    QObject::connect(_pimpl->upgraderWatcher.replica, &FirmwareUpgraderWatcherReplica::deviceScannerFinished,
                     this, &EdgeFirmwareUpgrader::_onDeviceScanningFinished);

    QObject::connect(_pimpl->upgraderWatcher.replica, &FirmwareUpgraderWatcherReplica::flasherFinished,
                     this, &EdgeFirmwareUpgrader::_onFlashingFinished);

    emit _setFilterParams(EdgeFirmwareUpgraderPrivate::EDGE_VID,
                          EdgeFirmwareUpgraderPrivate::EDGE_PIDS);
    emit _runRpiboot();
}


void EdgeFirmwareUpgrader::_onRpiBootFinished(bool status)
{
    if (status) {
        emit started();
    } else {
        emit startingFailed();
    }
}


void EdgeFirmwareUpgrader::_onDeviceScanningFinished(bool status)
{
    Q_UNUSED(status);
}


void EdgeFirmwareUpgrader::_onFlashingFinished(bool status)
{
    emit flashingCompleted();
}


void EdgeFirmwareUpgrader::_connectToProcess()
{
    auto successful = _pimpl->upgraderWatcher
            .serverNode
            .connectToNode(EdgeFirmwareUpgraderPrivate::FW_UPGRADER_NODE_URL);

    if (!successful) {
        emit errorMessageReceived("Can not connect to firmware upgrader process");
        return;
    }

    _pimpl->upgraderWatcher.replica =
            _pimpl->upgraderWatcher
            .serverNode
            .acquire<FirmwareUpgraderWatcherReplica>();

    QObject::connect(_pimpl->upgraderWatcher.replica, &FirmwareUpgraderWatcherReplica::initialized,
                     this, &EdgeFirmwareUpgrader::_connectToProcessSignals);
}


void EdgeFirmwareUpgrader::_runProcess()
{
    auto processName = EdgeFirmwareUpgraderPrivate::GRAPHICAL_SUDO_COMMAND +
                       EdgeFirmwareUpgraderPrivate::FW_UPGRADER_BINARY;

    _pimpl->upgraderWatcher.process.start(processName);
    _pimpl->upgraderWatcher.process.waitForStarted();

    QObject::connect(&_pimpl->upgraderWatcher.process, &QProcess::errorOccurred,
                     this, &EdgeFirmwareUpgrader::_onProcessErrorOcurred);

    QObject::connect(&_pimpl->upgraderWatcher.process, &QProcess::stateChanged,
                     this, &EdgeFirmwareUpgrader::_onProcessStateChanged);
}


void EdgeFirmwareUpgrader::_onProcessErrorOcurred(QProcess::ProcessError err)
{
    switch(err) {
        case QProcess::FailedToStart:
            qCritical("Failed to start firmware upgrader process");
            break;

        case QProcess::Crashed:
            qCritical("Firmware upgrader process crashed");
            break;

        case QProcess::UnknownError: default:
            qCritical("Unknown errror (Internal error)");
    }
}


void EdgeFirmwareUpgrader::_onProcessStateChanged(QProcess::ProcessState state)
{
    switch(state) {
        case QProcess::NotRunning:
            qInfo("Firmware upgrader not running.");
            break;

        case QProcess::Starting:
            qInfo("Firmware upgrader starting.");
            break;

        case QProcess::Running:
            qInfo("Firmware upgrader running");
            break;
    }
}


void EdgeFirmwareUpgrader::cancel(void)
{
    _pimpl->upgraderWatcher.replica->cancel();
}


void EdgeFirmwareUpgrader::enableChecksum(bool checksumEnabled)
{
    _pimpl->fwUpgraderParams.checksumEnabled = checksumEnabled;
}


bool EdgeFirmwareUpgrader::checksumEnabled(void) const
{
    return _pimpl->fwUpgraderParams.checksumEnabled;
}
