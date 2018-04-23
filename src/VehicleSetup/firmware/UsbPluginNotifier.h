#ifndef DEVICEOBSERVER_H
#define DEVICEOBSERVER_H

#include <functional>
#include <QtCore>

class UsbPluginNotifier : public QObject
{
    Q_OBJECT
public:
    using DeviceAvailablePredicate_t = std::function<bool(void)>;

    explicit UsbPluginNotifier(int const& vid,
                               QList<int> const& pids,
                               int interval = 1 ,
                               QObject *parent = nullptr);

    void setInterval(int msec) { _pollingTimer.setInterval(msec); }
    bool deviceAvailable(void) const;

signals:
    void devicePlugged   (void);
    void deviceUnplugged (void);

public slots:
    void observe(void);
    void stop(void);

private slots:
    void _onTimeout(void);

private:
    int _vid;
    QList<int> _pids;

    QTimer _pollingTimer;
    bool   _deviceWasAvailable;
};

#endif // DEVICEOBSERVER_H
