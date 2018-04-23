#include "UsbPluginNotifier.h"

#ifdef Q_OS_WIN
#pragma warning(push)
#pragma warning(disable:4200)
#include <libusb-1.0/libusb.h>
#pragma warning(pop)
#else
#include <libusb-1.0/libusb.h>
#endif


UsbPluginNotifier::UsbPluginNotifier(int const& vid,
                                     QList<int> const& pids,
                                     int interval,
                                     QObject *parent)
    : QObject(parent),
      _vid(vid),
      _pids(pids),
      _deviceWasAvailable(false)
{
    _pollingTimer.setInterval(interval);
    _pollingTimer.setSingleShot(false);

    QObject::connect(&_pollingTimer, &QTimer::timeout,
                     this,           &UsbPluginNotifier::_onTimeout);
}


void UsbPluginNotifier::observe(void)
{
    _pollingTimer.start();
}


bool UsbPluginNotifier::deviceAvailable(void) const
{
    libusb_context* ctx = nullptr;

    auto bootable = 0;
    auto ret = libusb_init(&ctx);

    libusb_device **list = nullptr;
    auto count = libusb_get_device_list(ctx, &list);

    Q_ASSERT(ret == 0 && count > 0);

    for (auto idx = 0; idx < count; ++idx) {
        auto usbDevice = list[idx];
        auto devDescr = libusb_device_descriptor{};

        ret = libusb_get_device_descriptor(usbDevice, &devDescr);

        if (devDescr.idVendor == _vid && _pids.contains(devDescr.idProduct)) {
            bootable++;
        }
    }

    libusb_exit(ctx);

    return bootable > 0;
}


void UsbPluginNotifier::stop(void)
{
    _pollingTimer.stop();
    _deviceWasAvailable = false;
}


void UsbPluginNotifier::_onTimeout(void)
{
    if (!deviceAvailable()) {
       if (_deviceWasAvailable) {
           emit deviceUnplugged();
           _deviceWasAvailable = false;
       }
    } else if (!_deviceWasAvailable) {
        emit devicePlugged();
        _deviceWasAvailable = true;
    }
}
