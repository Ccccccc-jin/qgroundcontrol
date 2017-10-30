#ifndef FIRMWAREUPGRADER_H
#define FIRMWAREUPGRADER_H

#include <QtCore>

#include "FirmwareImage.h"

class FirmwareUpgrader : public QObject
{
    Q_OBJECT

public:
    static FirmwareUpgrader* instance(void);

    virtual void start(void) = 0;

    virtual void cancel(void) = 0;

    virtual void reboot(void) = 0;

    virtual void flash(const FirmwareImage* image) = 0;

    virtual FirmwareImage* image(void) const = 0;

signals:
    void foundDevice(QString const& deviceName);

    void deviceNotFound(void);

    void error(QString const& errorString);

    void upgradingStatus(QString const& status);

    void upgradingCompleted(void);

    void upgradingProgressChanged(uint value);

protected:
    virtual uint _getDeviceTypeIfPresent(void);

    FirmwareUpgrader(QObject* parent = nullptr);

    virtual ~FirmwareUpgrader(void) = 0;
};

#endif // FIRMWAREUPGRADER_H
