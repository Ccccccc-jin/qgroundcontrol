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
    virtual ~FirmwareUpgraderClient(void) override;

    bool deviceAvailable(void)            const override;
    void flash(FlasherParameters const& params) override;

public slots:
    void initializeDevice (void) override;
    void cancel           (void) override;

private slots:
    void _startProcess   (void);
    void _initWatcher    (void);
    void _onWatcherReady (void);

    void _disconnectTmpConnections(void);
    void _onDeviceMountpointsAvailable(QStringList mountpoints);

signals:
    void _watcherReady(void);

private:
    bool _watcherInitialized     (void);
    void _initConnections        (void);
    void _attachToMessageHandler (void);
    void _attachToWatcher        (void);
    void _defineDeviceInitOrder  (void);

    QString _extractFirmwareVersion(QString const& bootPath);

    static const int        EDGE_VID;
    static const QList<int> EDGE_PIDS;
    static const QString    GRAPHICAL_SUDO_CMD_NAME;
    static const QString    SERVER_NODE_NAME;
    static const QString    EDGE_VERSION_FILE;

    const QString    FW_UPG_BINARY_FILENAME;

    MessageHandler      _messageHandler;

    QList<QMetaObject::Connection>     _temporaryConnections;
    std::shared_ptr<FirmwareUpgraderWatcherReplica> _watcher;
    QRemoteObjectNode _node;
};

#endif // FIRMWAREUPGRADERCLIENT_H
