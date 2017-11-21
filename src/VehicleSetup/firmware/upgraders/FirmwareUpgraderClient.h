#ifndef FIRMWAREUPGRADERCLIENT_H
#define FIRMWAREUPGRADERCLIENT_H

#include <QObject>

#include "FlasherParameters.h"
#include "MessageHandler.h"
#include "ProcessStateLog.h"
#include "firmware/FirmwareUpgraderInterface.h"

class FirmwareUpgraderClient : public FirmwareUpgrader
{
    Q_OBJECT
public:
    explicit FirmwareUpgraderClient(QObject *parent = nullptr);
    virtual ~FirmwareUpgraderClient(void);

    bool deviceAvailable(void) const override;

    void flash(FlasherParameters const& params) override;

public slots:
    void cancel(void) override;
    void finish(void) override;
    void start (void) override;

private slots:
    void _startProcess (void);
    void _initWatcher  (void);

    void _onWatcherInitialized(void);
    void _onDeviceMountpointsAvailable(QStringList mountpoints);

private:
    void _initConnections (void);
    void _attachToWatcher (void);
    void _attachToProcess (void);

    void _attachToMessageHandler          (void);
    void _finalizeFirmwareUpgraderProcess (void);

    QString _edgeVersionExtractor(QString const& bootPath);

    static const int        EDGE_VID;
    static const QList<int> EDGE_PIDS;
    static const QString    GRAPHICAL_SUDO_BIN;
    static const QString    SERVER_NODE_NAME;
    static const QString    EDGE_VERSION_FILE;

    const QString    FW_UPG_BINARY_FILE;

    MessageHandler      _messageHandler;
    ProcessStateLog     _processLog;

    std::shared_ptr<FirmwareUpgraderWatcherReplica> _watcher;
    QRemoteObjectNode _node;
    QProcess _fwUpgProcess;
};

#endif // FIRMWAREUPGRADERCLIENT_H
