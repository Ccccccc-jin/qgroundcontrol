#include "FirmwareUpgraderClient.h"
#include <algorithm>

#ifdef Q_OS_WIN
#pragma warning(push)
#pragma warning(disable:4200)
#include <libusb-1.0/libusb.h>
#pragma warning(pop)
#else
#include <libusb-1.0/libusb.h>
#endif


FirmwareUpgraderClient::FirmwareUpgraderClient(UpdateConfig const& config,
                                               QObject *parent)
    : FirmwareUpgrader(parent),
      _config(config)
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
    auto const requiredVid  = _config.edgeVid();
    auto const requiredPids = _config.edgePids();

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

        _updaterServer->closeSession();
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

    _updaterServer->closeSession();
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

    QProcess::startDetached(cmdexe, {cmdRunCommandKey, _config.fwUpdaterBinaryPath()});
#elif defined(Q_OS_MACX)
    auto const graphicalSudo = "osascript";
    auto const arg = "-e";
    auto const script = QString("do shell script \"%1\" with administrator privileges")
        .arg(_config.fwUpdaterBinaryPath());

    QProcess::startDetached(graphicalSudo, {arg, script});
#elif defined(Q_OS_LINUX)
    auto env = QProcessEnvironment::systemEnvironment();
    auto appimageVarName = "APPIMAGE";

    auto shellExecuteCmd = [] (QString const& args) {
        auto shell = "/bin/sh";
        QProcess::startDetached(shell, {"-c", args});
    };

    auto makeArgs = [] (QStringList&& args) {
        std::transform(args.begin(), args.end(), args.begin(),
                       [] (QString& arg) { return arg.append('\"').prepend('\"'); });

        return args.join(" ");
    };

    auto shellArgs = QString();

    /* if QGroung was started from AppImage */
    if (env.contains(appimageVarName)) {
        auto appimagePath  = env.value(appimageVarName);
        auto fwUpgraderKey = "--fwupg";

        shellArgs = makeArgs({GRAPHICAL_SUDO_CMD_NAME, appimagePath, fwUpgraderKey});
    } else {
        if (QCoreApplication::applicationDirPath().isEmpty()) {
            auto warnMessage = QString("Can not start firmware upgrading.") +
                     "You should start the program through qgroundcontrol-start.sh";
            emit warnMessageReceived(warnMessage);
            return;
        }

        shellArgs = makeArgs({GRAPHICAL_SUDO_CMD_NAME, _config.fwUpdaterBinaryPath()});
    }

    shellExecuteCmd(shellArgs);
#endif
}


void FirmwareUpgraderClient::_initUpdater(void)
{
    auto successful = _clientNode
            .connectToNode(_config.serverNodeName());

    if (!successful) {
        qCritical() << "Can not connect to server node";
        emit errorMessageReceived("Can not connect to firmware upgrader process");
        emit deviceInitialized(false);
        return;
    }

    using Updater = EdgeFirmwareUpdaterIPCReplica;
    using FwUpg   = FirmwareUpgraderClient;

    _updaterServer.reset(_clientNode.acquire<Updater>());
    QObject::connect(_updaterServer.get(), &Updater::initialized,
                     this, &FwUpg::_attachToUpdater);
}


void FirmwareUpgraderClient::_onUpdaterReady(void)
{
    _updaterServer->openSession();

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
