#include <libusb-1.0/libusb.h>
#include "FirmwareUpgraderClient.h"


int const         FirmwareUpgraderClient::EDGE_VID                = 0x0a5c;
QList<int> const  FirmwareUpgraderClient::EDGE_PIDS               = QList<int>({0x2763, 0x2764});
QString const     FirmwareUpgraderClient::GRAPHICAL_SUDO_CMD_NAME = "pkexec";
QString const     FirmwareUpgraderClient::SERVER_NODE_NAME        = "local:fwupg_socket";
QString const     FirmwareUpgraderClient::EDGE_VERSION_FILE       = "/issue.txt";


FirmwareUpgraderClient::FirmwareUpgraderClient(QObject *parent)
    : FirmwareUpgrader(parent),
      FW_UPG_BINARY_FILENAME(QCoreApplication::applicationDirPath() + "/bin/fwupgrader-start.sh")
{
    _initConnections();
}


FirmwareUpgraderClient::~FirmwareUpgraderClient(void)
{
    if (_watcherInitialized()) {
        qDebug() << "finish firmwareupgrader  process";
        _watcher->finish();
    }

    while(_watcherInitialized()) {
        QCoreApplication::processEvents();
    }

    qDebug() << "Firmware upgrader client destroyed";
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


void FirmwareUpgraderClient::flash(FlasherParameters const& params)
{
    using Watcher = FirmwareUpgraderWatcherReplica;
    using FwUpg   = FirmwareUpgraderClient;

    QMetaObject::Connection connection;

    if (params.checksumEnabled()) {
        connection = QObject::connect(_watcher.get(), &Watcher::flasherFinished,
            [this] (bool status) { if (status) { _watcher->runCheckingCorrectnessStep(); } }
        );
        _temporaryConnections.append(connection);

        connection = QObject::connect(_watcher.get(), &Watcher::checkingCorrectnessFinished, this, &FwUpg::deviceFlashed);
        _temporaryConnections.append(connection);

    } else {
        QObject::connect(_watcher.get(), &Watcher::flasherFinished, this, &FwUpg::deviceFlashed);
        _temporaryConnections.append(connection);
    }

    _watcher->runFlasherStep(params.image().filename());
}


void FirmwareUpgraderClient::initializeDevice(void)
{
    if (!_watcherInitialized()) {
        qDebug() << "watcher not initialized. Start new process";
        _startProcess();
        _initWatcher();
        return;
    }

    emit _watcherReady();
}


void FirmwareUpgraderClient::cancel(void)
{
    _watcher->cancel();
}


void FirmwareUpgraderClient::_startProcess(void)
{
    auto env = QProcessEnvironment::systemEnvironment();
    auto appimageVarName = "APPIMAGE";

    /* if QGroung was started from AppImage */
    if (env.contains(appimageVarName)) {
        auto appimagePath  = env.value(appimageVarName);
        auto fwUpgraderKey = "--fwupg";

        QProcess::startDetached(GRAPHICAL_SUDO_CMD_NAME, { appimagePath, fwUpgraderKey });
    } else {
        if (QCoreApplication::applicationDirPath().isEmpty()) {
            auto warnMessage = QString("Can not start firmware upgrading.") +
                     "You should start the program through qgroundcontrol-start.sh";
            emit warnMessageReceived(warnMessage);
            return;
        }

        QProcess::startDetached(GRAPHICAL_SUDO_CMD_NAME, { FW_UPG_BINARY_FILENAME });
    }
}


void FirmwareUpgraderClient::_initWatcher(void)
{
    auto successful = _node.connectToNode(SERVER_NODE_NAME);

    if (!successful) {
        qCritical() << "Can not connect to server node";
        emit errorMessageReceived("Can not connect to firmware upgrader process");
        emit deviceInitialized(false);
        return;
    }

    using Watcher = FirmwareUpgraderWatcherReplica;
    using FwUpg   = FirmwareUpgraderClient;

    _watcher.reset(_node.acquire<Watcher>());
    QObject::connect(_watcher.get(), &Watcher::initialized, this, &FwUpg::_attachToWatcher);
}


void FirmwareUpgraderClient::_onWatcherReady(void)
{
    emit deviceInitializationStarted();
    _watcher->setVidPid(EDGE_VID, EDGE_PIDS);
    _watcher->runRpiBootStep();
}


void FirmwareUpgraderClient::_disconnectTmpConnections(void)
{
    for(auto const& connection : _temporaryConnections) {
        QObject::disconnect(connection);
    }

    _temporaryConnections.clear();
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

    QString version = _extractFirmwareVersion(bootPath);
    qInfo() << "Boot path: "  << bootPath;
    qInfo() << "Fw version: " << version;

    emit firmwareVersionAvailable(version);
}


bool FirmwareUpgraderClient::_watcherInitialized(void)
{
    return _watcher != nullptr && _watcher->isReplicaValid();
}


void FirmwareUpgraderClient::_initConnections(void)
{
    QObject::connect(this, &FirmwareUpgraderClient::_watcherReady,
                     this, &FirmwareUpgraderClient::_onWatcherReady);

    QObject::connect(this, &FirmwareUpgraderClient::deviceFlashed,
                    [this] (bool status) { Q_UNUSED(status); _disconnectTmpConnections(); } );

    _attachToMessageHandler();
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
    qInfo() << "Watcher initialized.";

    using Watcher = FirmwareUpgraderWatcherReplica;
    using FwUpg   = FirmwareUpgraderClient;

    auto watcherPtr = _watcher.get();

    _defineDeviceInitOrder();

    QObject::connect(watcherPtr, &Watcher::flasherProgressChanged,             this, &FwUpg::progressChanged);
    QObject::connect(watcherPtr, &Watcher::checkingCorrectnessProgressChanged, this, &FwUpg::progressChanged);
    QObject::connect(watcherPtr, &Watcher::deviceMountpoints,                  this, &FwUpg::_onDeviceMountpointsAvailable);
    QObject::connect(watcherPtr, &Watcher::cancelled,                          this, &FwUpg::cancelled);

    _messageHandler.attach(_watcher);
    emit _watcherReady();
}


void FirmwareUpgraderClient::_defineDeviceInitOrder(void)
{
    using Watcher = FirmwareUpgraderWatcherReplica;

    auto watcherPtr = _watcher.get();

    QObject::connect(watcherPtr, &Watcher::rpiBootFinished,
        [this] (bool status) {
            if (status) {
                _watcher->runDeviceScannerStep();
            } else {
                emit deviceInitialized(false);
            }
        }
    );

    QObject::connect(watcherPtr, &Watcher::deviceScannerFinished,
        [this] (bool status) {
            if (status) {
                emit deviceInitialized(true);
            } else {
                emit deviceInitialized(false);
            }
        }
    );
}


QString FirmwareUpgraderClient::_extractFirmwareVersion(QString const& bootPath)
{
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
