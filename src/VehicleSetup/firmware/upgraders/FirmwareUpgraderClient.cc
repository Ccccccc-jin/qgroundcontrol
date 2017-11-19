#include "FirmwareUpgraderClient.h"
#include <libusb-1.0/libusb.h>


int const         FirmwareUpgraderClient::EDGE_VID = 0x0a5c;
QList<int> const  FirmwareUpgraderClient::EDGE_PIDS = QList<int>({0x2763, 0x2764});
QString const     FirmwareUpgraderClient::FW_UPG_BINARY_FILE =
                     "/home/vladimir.provalov/dev/qgcflasher/build-firmwareupgrader-fwupgrader-Debug/fwupgrader";
QString const     FirmwareUpgraderClient::GRAPHICAL_SUDO_BIN = "pkexec";
QString const     FirmwareUpgraderClient::SERVER_NODE_NAME   = "local:fwupg_socket";
QString const     FirmwareUpgraderClient::EDGE_VERSION_FILE  = "/issue.txt";


FirmwareUpgraderClient::FirmwareUpgraderClient(QObject *parent)
    : FirmwareUpgrader(parent)
{
    _attachToMessageHandler();
    _attachToProcess();
    _processLog.attach(_fwUpgProcess);
}


FirmwareUpgraderClient::~FirmwareUpgraderClient(void)
{
    if (_watcher) {
        _watcher->finish();
    }

    if (_fwUpgProcess.state() == QProcess::Running) {
        _fwUpgProcess.kill();
        _fwUpgProcess.waitForFinished();
    }
}


void FirmwareUpgraderClient::_attachToProcess(void)
{
    QObject::connect(&_fwUpgProcess,
        static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
        [this](int exitCode, QProcess::ExitStatus stat) {
            Q_UNUSED(exitCode);
            Q_UNUSED(stat);
            emit finished();
        }
    );
}


void FirmwareUpgraderClient::_attachToMessageHandler(void)
{
    using MsgHandler = MessageHandler;
    using FwUpg = FirmwareUpgraderClient;

    QObject::connect(&_messageHandler, &MsgHandler::errorMessageReceived,
                     this,             &FwUpg::errorMessageReceived);

    QObject::connect(&_messageHandler, &MsgHandler::infoMessageReceived,
                     this,             &FwUpg::infoMessageReceived);

    QObject::connect(&_messageHandler, &MsgHandler::warnMessageReceived,
                     this,             &FwUpg::warnMessageReceived);

    QObject::connect(&_fwUpgProcess, &QProcess::started,
                     this,           &FwUpg::_initWatcher);
}


void FirmwareUpgraderClient::_attachToWatcher(void)
{
    using Watcher = FirmwareUpgraderWatcherReplica;
    using FwUpg   = FirmwareUpgraderClient;

    auto watcherPtr = _watcher.get();

    QObject::connect(watcherPtr, &Watcher::rpiBootFinished,
                     this,       &FwUpg::_onRpiBootFinished);

    QObject::connect(watcherPtr, &Watcher::deviceScannerFinished,
                     this,       &FwUpg::_onDeviceScannerFinished);

    QObject::connect(watcherPtr, &Watcher::flasherFinished,
                     this,       &FwUpg::_onFlasherFinished);

    QObject::connect(watcherPtr, &Watcher::flasherProgressChanged,
                     this,       &FwUpg::progressChanged);

    QObject::connect(watcherPtr, &Watcher::deviceMountpoints,
                     this,       &FwUpg::_onDeviceMountpointsAvailable);

    QObject::connect(watcherPtr, &Watcher::cancelled,
                     this,       &FwUpg::cancelled);

    QObject::connect(watcherPtr, &Watcher::finished,
                     this,       &FwUpg::finished);

    _messageHandler.attach(_watcher);
}


bool FirmwareUpgraderClient::deviceAvailable() const
{
    auto const requiredVid  = FirmwareUpgraderClient::EDGE_VID;
    auto const requiredPids = FirmwareUpgraderClient::EDGE_PIDS;

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


void FirmwareUpgraderClient::start(void)
{
    _startProcess();
}


void FirmwareUpgraderClient::_startProcess(void)
{
    using FwUpg = FirmwareUpgraderClient;

    _fwUpgProcess.start(FwUpg::GRAPHICAL_SUDO_BIN + " " +
                        FwUpg::FW_UPG_BINARY_FILE);
}


void FirmwareUpgraderClient::_initWatcher(void)
{
    using FwUpg   = FirmwareUpgraderClient;
    using Watcher = FirmwareUpgraderWatcherReplica;

    auto successful = _node.connectToNode(FwUpg::SERVER_NODE_NAME);

    if (!successful) {
        qCritical() << "Can not connect to server node";
        emit errorMessageReceived("Can not connect to firmware upgrader process");
        return;
    }

    _watcher.reset(_node.acquire<Watcher>());

    QObject::connect(_watcher.get(), &Watcher::initialized,
                     this,           &FwUpg::_onWatcherInitialized);
}


void FirmwareUpgraderClient::_onWatcherInitialized(void)
{
    // attach all clients
    qInfo() << "Watcher initialized.";
    _attachToWatcher();

    _watcher->setFilterParams(FirmwareUpgraderClient::EDGE_VID,
                              FirmwareUpgraderClient::EDGE_PIDS);

    _watcher->runRpiBootStep();
}


QString FirmwareUpgraderClient::_edgeVersionExtractor(QString const& bootPath) {
    QString issuePath(bootPath + FirmwareUpgraderClient::EDGE_VERSION_FILE);
    QFile issueFile(issuePath);

    auto successful = issueFile.open(QIODevice::ReadOnly);

    if (!successful) {
        qCritical() << "Failed to open: " << issuePath;
        return QString();
    }

    QRegExp regexp("^v\\d\\.\\d");

    while (!issueFile.atEnd()) {
        auto line = issueFile.readLine();
        auto pos = 0;

        if (regexp.indexIn(line, pos) != -1) {
            return regexp.capturedTexts().at(0);
        }
    }

    qCritical() << "This file doesn't contain edge version.";

    return QString();
}


void FirmwareUpgraderClient::
    _onDeviceMountpointsAvailable(QStringList mountpoints)
{
    QString bootPath;

    for (auto mnt : mountpoints) {
        if (mnt.contains("boot")) {
            bootPath = std::move(mnt);
            break;
        }
    }

    qInfo() << "Boot path: " << bootPath;
    QString version = _edgeVersionExtractor(bootPath);
    qInfo() << "Fw version: " << version;

    emit firmwareVersionAvailable(version);
}


void FirmwareUpgraderClient::_onRpiBootFinished(bool status)
{
    if (!status) {
        qDebug() << "RpiBoot failed.";
        return;
    }

    _watcher->runDeviceScannerStep();
}


void FirmwareUpgraderClient::_onDeviceScannerFinished(bool status)
{
    if (!status) {
        qDebug() << "Device scanner failed.";
        return;
    }

    emit started();
}


void FirmwareUpgraderClient::_onFlasherFinished(bool status)
{
    if (!status) {
       qCritical() << "Flasher failed";
    }

    emit flashingFinished(status);
}


void FirmwareUpgraderClient::cancel(void)
{
    _watcher->cancel();
}


void FirmwareUpgraderClient::finish(void)
{
    _watcher->finish();
}


void FirmwareUpgraderClient::flash(FlasherParameters const& params)
{
   _watcher->runFlasherStep(params.image().filename(), params.checksumEnabled());
}
