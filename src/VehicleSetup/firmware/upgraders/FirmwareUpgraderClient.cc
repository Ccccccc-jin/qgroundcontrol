#include <libusb-1.0/libusb.h>
#include "FirmwareUpgraderClient.h"


int const         FirmwareUpgraderClient::EDGE_VID                = 0x0a5c;
QList<int> const  FirmwareUpgraderClient::EDGE_PIDS               = QList<int>({0x2763, 0x2764});
QString const     FirmwareUpgraderClient::GRAPHICAL_SUDO_CMD_NAME = "pkexec";
QString const     FirmwareUpgraderClient::SERVER_NODE_NAME        = "local:fwupg_socket";
QString const     FirmwareUpgraderClient::EDGE_VERSION_FILE       = "/issue.txt";


FirmwareUpgraderClient::FirmwareUpgraderClient(QObject *parent)
    : FirmwareUpgrader(parent)
{
    _initConnections();
}


FirmwareUpgraderClient::~FirmwareUpgraderClient(void)
{
    if (_updaterInitialized()) {
        qDebug() << "finish firmwareupgrader  process";
        _updaterServer->finish();
    }

    while(_updaterInitialized()) {
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
    using Updater = EdgeFirmwareUpdaterIPCReplica;

    auto deviceFlashedHandle = [this] (int status) {
        auto stat = static_cast<FinishStatus>(status);
        if (stat == FinishStatus::Succeed) {
            emit deviceFlashed(true);
        } else if (stat == FinishStatus::Failed) {
            emit deviceFlashed(false);
        }
    };

    if (params.checksumEnabled()) {
        auto connection = QObject::connect(_updaterServer.get(), &Updater::flashingFinished,
            [this] (int status) {
                if (status == FinishStatus::Succeed) {
                    _updaterServer->checkOnCorrectness();
                } else if (status == FinishStatus::Failed) {
                    emit deviceFlashed(false);
                }
            }
        );
        _temporaryConnections.append(connection);

        connection = QObject::connect(_updaterServer.get(), &Updater::checkOnCorrectnessFinished,
                                      deviceFlashedHandle);
        _temporaryConnections.append(connection);

    } else {
        auto connection = QObject::connect(_updaterServer.get(), &Updater::flashingFinished,
                                           deviceFlashedHandle);

        _temporaryConnections.append(connection);
    }

    _updaterServer->flash(params.image().filename());
}


void FirmwareUpgraderClient::initializeDevice(void)
{
    _disconnectTmpConnections();
    if (!_updaterInitialized()) {
        qDebug() << "watcher not initialized. Start new process";
        _startProcess();
        _initUpdater();
        return;
    }

    emit _updaterReady();
}


void FirmwareUpgraderClient::cancel(void)
{
    _updaterServer->cancel();
}


void FirmwareUpgraderClient::_startProcess(void)
{
#ifdef Q_OS_WIN
   auto const cmdexe = QString("cmd.exe");
   auto const cmdRunCommandKey = QString("/C");
   auto const fwUpgBinaryFile  = _fwUpgraderBinaryFilename();

   QProcess::startDetached(cmdexe, {cmdRunCommandKey, fwUpgBinaryFile});
#else
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

        QProcess::startDetached(GRAPHICAL_SUDO_CMD_NAME, { _fwUpgraderBinaryFilename() });
    }
#endif
}


void FirmwareUpgraderClient::_initUpdater(void)
{
    auto successful = _clientNode.connectToNode(SERVER_NODE_NAME);

    if (!successful) {
        qCritical() << "Can not connect to server node";
        emit errorMessageReceived("Can not connect to firmware upgrader process");
        emit deviceInitialized(false);
        return;
    }

    using Updater = EdgeFirmwareUpdaterIPCReplica;
    using FwUpg   = FirmwareUpgraderClient;

    _updaterServer.reset(_clientNode.acquire<Updater>());
    QObject::connect(_updaterServer.get(), &Updater::initialized, this, &FwUpg::_attachToUpdater);
}


void FirmwareUpgraderClient::_onUpdaterReady(void)
{
    using Updater = EdgeFirmwareUpdaterIPCReplica;
    auto connection = QObject::connect(_updaterServer.get(), &Updater::initializingFinished,
        [this] (int status) {
            if (status == FinishStatus::Succeed) {
                emit deviceInitialized(true);
            } else {
                emit deviceInitialized(false);
            }
        }
    );

    _temporaryConnections.append(connection);

    _updaterServer->initializeEdgeDevice();
    emit deviceInitializationStarted();
}


void FirmwareUpgraderClient::_disconnectTmpConnections(void)
{
    for(auto const& connection : _temporaryConnections) {
        QObject::disconnect(connection);
    }

    _temporaryConnections.clear();
}


QString FirmwareUpgraderClient::_fwUpgraderBinaryFilename(void)
{
    auto genericFilename = QCoreApplication::applicationDirPath() + "/fwupgrader";

#ifdef Q_OS_WIN
    return genericFilename.append(".exe").replace("/","\\");
#else
    return genericFilename;
#endif
}


void FirmwareUpgraderClient::_handleMessage(QString msg, int type)
{
    switch (static_cast<LogMsgType>(type)) {
        case LogMsgType::Info: {
            emit infoMessageReceived(msg);
            break;
        }

        case LogMsgType::Warning: {
            emit warnMessageReceived(msg);
            break;
        }

        case LogMsgType::Error: {
            emit errorMessageReceived(msg);
            break;
        }

        default: {
            qCritical() << "unsupported msg type";
        }
    }
}


bool FirmwareUpgraderClient::_updaterInitialized(void)
{
    return _updaterServer != nullptr && _updaterServer->isReplicaValid();
}


void FirmwareUpgraderClient::_initConnections(void)
{
    QObject::connect(this, &FirmwareUpgraderClient::_updaterReady,
                     this, &FirmwareUpgraderClient::_onUpdaterReady);

}


void FirmwareUpgraderClient::_attachToUpdater(void)
{
    qInfo() << "Updater initialized.";

    using Updater = EdgeFirmwareUpdaterIPCReplica;
    using FwUpg   = FirmwareUpgraderClient;

    auto updaterPtr = _updaterServer.get();

    QObject::connect(updaterPtr, &Updater::progressChanged, this, &FwUpg::progressChanged);
    QObject::connect(updaterPtr, &Updater::cancelled,       this, &FwUpg::cancelled);
    QObject::connect(updaterPtr, &Updater::firmwareVersion, this, &FwUpg::firmwareVersionAvailable);
    QObject::connect(updaterPtr, &Updater::logMessage,      this, &FwUpg::_handleMessage);

    emit _updaterReady();
}
