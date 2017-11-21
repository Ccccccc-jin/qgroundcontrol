#include "FirmwareUpgraderClient.h"
#include <libusb-1.0/libusb.h>


int const         FirmwareUpgraderClient::EDGE_VID           = 0x0a5c;
QList<int> const  FirmwareUpgraderClient::EDGE_PIDS          = QList<int>({0x2763, 0x2764});
QString const     FirmwareUpgraderClient::GRAPHICAL_SUDO_BIN = "pkexec";
QString const     FirmwareUpgraderClient::SERVER_NODE_NAME   = "local:fwupg_socket";
QString const     FirmwareUpgraderClient::EDGE_VERSION_FILE  = "/issue.txt";


FirmwareUpgraderClient::FirmwareUpgraderClient(QObject *parent)
    : FirmwareUpgrader(parent),
      FW_UPG_BINARY_FILE(QCoreApplication::applicationDirPath() + "/bin/fwupgrader")
{
    _initConnections();
}


FirmwareUpgraderClient::~FirmwareUpgraderClient(void)
{
    _finalizeFirmwareUpgraderProcess();
}


void FirmwareUpgraderClient::_initConnections(void)
{
    using FwUpg   = FirmwareUpgraderClient;
    // after our process started, we need to initialise "watcher" replica
    QObject::connect(&_fwUpgProcess, &QProcess::started, this, &FwUpg::_initWatcher);

    // attach other
    _attachToMessageHandler();
    _attachToProcess();
    _processLog.attach(_fwUpgProcess);
}


void FirmwareUpgraderClient::_attachToProcess(void)
{
    QObject::connect(&_fwUpgProcess,
        static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
        [this](int exitCode, QProcess::ExitStatus stat) {
            Q_UNUSED(exitCode); Q_UNUSED(stat);
            emit finished();
        }
    );
}


void FirmwareUpgraderClient::_attachToMessageHandler(void)
{
    using MsgHandler = MessageHandler;
    using FwUpg      = FirmwareUpgraderClient;

    QObject::connect(&_messageHandler, &MsgHandler::errorMessageReceived, this, &FwUpg::errorMessageReceived);
    QObject::connect(&_messageHandler, &MsgHandler::infoMessageReceived,  this, &FwUpg::infoMessageReceived);
    QObject::connect(&_messageHandler, &MsgHandler::warnMessageReceived,  this, &FwUpg::warnMessageReceived);
}


void FirmwareUpgraderClient::_attachToWatcher(void)
{
    using Watcher = FirmwareUpgraderWatcherReplica;
    using FwUpg   = FirmwareUpgraderClient;

    auto watcherPtr = _watcher.get();

    QObject::connect(watcherPtr, &Watcher::flasherProgressChanged, this, &FwUpg::progressChanged);
    QObject::connect(watcherPtr, &Watcher::deviceMountpoints,      this, &FwUpg::_onDeviceMountpointsAvailable);
    QObject::connect(watcherPtr, &Watcher::flasherFinished,        this, &FwUpg::flashingFinished);
    QObject::connect(watcherPtr, &Watcher::finished,               this, &FwUpg::finished);

    // after watcher initilized, we need to run rpiboot in _onWatcherInitilized slot
    QObject::connect(watcherPtr, &Watcher::initialized,     this, &FwUpg::_onWatcherInitialized);
    // after rpiboot finished, we need to run device scanner(which return device mountpoints)
    QObject::connect(watcherPtr, &Watcher::rpiBootFinished,
        [this] (bool status) {
            qInfo() << "Rpiboot status: " << status;
            if (status) _watcher->runDeviceScannerStep();
        }
    );
    // after device scanning firmware upgrader is ready to flash device (emit 'ready' signal)
    QObject::connect(watcherPtr, &Watcher::deviceScannerFinished,
        [this] (bool status) {
            qInfo() << "DeviceScanner status: " << status;
            if (status) emit ready();
        }
    );

    _messageHandler.attach(_watcher);
}


void FirmwareUpgraderClient::_finalizeFirmwareUpgraderProcess(void)
{
    if (_watcher != nullptr && _watcher->isInitialized()) {
        _watcher->finish();
    }

    if (_fwUpgProcess.state() != QProcess::NotRunning) {
        _fwUpgProcess.kill();
        _fwUpgProcess.waitForFinished();
    }
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

    if (QCoreApplication::applicationDirPath().isEmpty()) {
        emit warnMessageReceived(QString("Can not start firmware upgrading.") +
                                 "You should start the program through qgroundcontrol-start.sh");
        return;
    }

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
    _attachToWatcher();
}


void FirmwareUpgraderClient::_onWatcherInitialized(void)
{
    // attach all clients
    emit initialzed();
    qInfo() << "Watcher initialized.";

    _watcher->setVidPid(FirmwareUpgraderClient::EDGE_VID,
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

    QString version = _edgeVersionExtractor(bootPath);
    qInfo() << "Boot path: "  << bootPath;
    qInfo() << "Fw version: " << version;

    emit firmwareVersionAvailable(version);
}


void FirmwareUpgraderClient::cancel(void)
{
    _finalizeFirmwareUpgraderProcess();
}


void FirmwareUpgraderClient::finish(void)
{
    _finalizeFirmwareUpgraderProcess();
}


void FirmwareUpgraderClient::flash(FlasherParameters const& params)
{
   _watcher->runFlasherStep(params.image().filename(), params.checksumEnabled());
}
