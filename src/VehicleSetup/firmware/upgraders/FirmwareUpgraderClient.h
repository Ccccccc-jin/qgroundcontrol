#ifndef FIRMWAREUPGRADERCLIENT_H
#define FIRMWAREUPGRADERCLIENT_H

#include <QObject>
#include <QtRemoteObjects>

#include "FlasherParameters.h"
#include "UpdateConfig.h"
#include "ProcessStateLog.h"
#include "firmware/FirmwareUpgraderInterface.h"
#include "rep_EdgeFirmwareUpdaterIPC_replica.h"

class FirmwareUpgraderClient : public FirmwareUpgrader
{
    Q_OBJECT
public:
    explicit FirmwareUpgraderClient(UpdateConfig const& config,
                                    QObject *parent = nullptr);
    virtual ~FirmwareUpgraderClient(void) override;

    bool deviceAvailable(void)            const override;
    void flash(FlasherParameters const& params) override;

public slots:
    void initializeDevice (void) override;
    void cancel           (void) override;

private slots:
    void _startProcess   (void);
    void _initUpdater    (void);
    void _onUpdaterReady (void);

    void _disconnectTmpConnections(void);

signals:
    void _updaterReady(void);

private:
    void _handleMessage(QString msg, int type);
    bool _updaterInitialized (void);
    void _initConnections    (void);
    void _attachToUpdater    (void);

    UpdateConfig _config;
    QList<QMetaObject::Connection>     _temporaryConnections;
    std::shared_ptr<EdgeFirmwareUpdaterIPCReplica> _updaterServer;
    QRemoteObjectNode _clientNode;
};

#endif // FIRMWAREUPGRADERCLIENT_H
