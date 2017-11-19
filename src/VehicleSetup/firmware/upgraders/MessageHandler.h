#ifndef EDGEMESSAGEHANDLER_H
#define EDGEMESSAGEHANDLER_H

#include <functional>
#include <QObject>
#include <memory>

#include "rep_FirmwareUpgraderWatcher_replica.h"

class MessageHandler;

using MessageSignal_t = std::function<void(QString const&)>;

class MessageHandler : public QObject
{
    Q_OBJECT
public:
    explicit MessageHandler(QObject *parent = nullptr);

    void attach(std::shared_ptr<FirmwareUpgraderWatcherReplica> watcher);

signals:
    void infoMessageReceived(QString const& msg);

    void warnMessageReceived(QString const& msg);

    void errorMessageReceived(QString const& msg);

public slots:
    void onRpiBootStateChanged(uint state, uint type);

    void onDeviceScannerStateChanged(uint state, uint type);

    void onFlasherStateChanged(uint state, uint type);

private:

    MessageSignal_t _getMessageSignal(uint messageType);
};

#endif // EDGEMESSAGEHANDLER_H
