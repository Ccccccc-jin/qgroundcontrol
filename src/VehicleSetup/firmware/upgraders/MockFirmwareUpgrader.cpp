#include "MockFirmwareUpgrader.h"

MockFirmwareUpgrader::MockFirmwareUpgrader(QObject *parent)
    : FirmwareUpgrader(parent),
      _timer(this),
      _progress(0)
{  }


MockFirmwareUpgrader::~MockFirmwareUpgrader(void)
{  }


void MockFirmwareUpgrader::cancel(void)
{
    _timer.stop();
    _progress = 0;

    emit flasherProgressChanged(_progress);
    emit infoMessageReceived("Flashing cancelled.");
}


void MockFirmwareUpgrader::reboot(void)
{
    _progress = 0;
    emit flasherProgressChanged(_progress);
    emit infoMessageReceived("Flashing rebooted");
}


void MockFirmwareUpgrader::flash(FirmwareImage *image)
{
    Q_UNUSED(image);

    _timer.setSingleShot(false);

    emit infoMessageReceived("Firmware Upgrading process started.");
    emit infoMessageReceived("RpiBoot completed.");
    emit infoMessageReceived("Rpi device has found.");
    emit infoMessageReceived("Start flashing device...");

    QObject::connect(&_timer, &QTimer::timeout,
        [this] () mutable {
            emit flasherProgressChanged(_progress);

            if (_progress == 100) {
                _timer.stop();
                emit infoMessageReceived("Flashing successfully completed.");
            } else {
                _progress += 1;
            }
        }
    );

    _timer.start(10000);
}


FirmwareImage* MockFirmwareUpgrader::image(void) const
{
   return nullptr;
}
