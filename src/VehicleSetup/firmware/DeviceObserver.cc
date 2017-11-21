#include "DeviceObserver.h"


DeviceObserver::DeviceObserver(int interval, QObject *parent)
    : QObject(parent),
      _deviceWasAvailable(false)
{
    _pollingTimer.setInterval(interval);
    _pollingTimer.setSingleShot(false);

    QObject::connect(&_pollingTimer, &QTimer::timeout,
                     this,           &DeviceObserver::_onTimeout);
}


void DeviceObserver::observe(void)
{
    _pollingTimer.start();
}


void DeviceObserver::stop(void)
{
    _pollingTimer.stop();
    _deviceWasAvailable = false;
}


void DeviceObserver::_onTimeout(void)
{
    if (!_deviceAvailable()) {
       if (_deviceWasAvailable) {
           emit deviceUnplugged();
           _deviceWasAvailable = false;
       }
    } else if (!_deviceWasAvailable) {
        emit devicePlugged();
        _deviceWasAvailable = true;
    }
}
