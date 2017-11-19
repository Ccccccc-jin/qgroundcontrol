#ifndef EDGEMESSAGEHANDLER_H
#define EDGEMESSAGEHANDLER_H

#include <functional>
#include <QObject>

class EdgeMessageHandler;

using MessageSignal_t = std::function<void(QString const&)>;

class EdgeMessageHandler : public QObject
{
    Q_OBJECT
public:
    explicit EdgeMessageHandler(QObject *parent = nullptr);


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
