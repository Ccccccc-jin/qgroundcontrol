#ifndef DEVICEOBSERVER_H
#define DEVICEOBSERVER_H

#include <functional>
#include <QtCore>

class DeviceObserver : public QObject
{
    Q_OBJECT
public:
    using DeviceAvailablePredicate_t = std::function<bool(void)>;

    explicit DeviceObserver(int interval = 1 , QObject *parent = nullptr);

    void setDeviceAvailablePredicate(DeviceAvailablePredicate_t pred) { _deviceAvailable = pred; }
    void setInterval(int msec) { _pollingTimer.setInterval(msec); }

signals:
    void devicePlugged   (void);
    void deviceUnplugged (void);

public slots:
    void observe(void);
    void stop(void);

private slots:
    void _onTimeout(void);

private:
    QTimer _pollingTimer;
    bool   _deviceWasAvailable;

    DeviceAvailablePredicate_t _deviceAvailable;
};

#endif // DEVICEOBSERVER_H
