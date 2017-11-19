#ifndef FIRMWAREUPGRADERINTERFACE_H
#define FIRMWAREUPGRADERINTERFACE_H

#include <QObject>
#include "FirmwareImage.h"
#include <memory>

class FirmwareUpgrader : public QObject
{
    Q_OBJECT
public:

    static std::unique_ptr<FirmwareUpgrader> instance(void);

    virtual ~FirmwareUpgrader(void);

    virtual bool deviceAvailable(void) const = 0;

    virtual void cancel(void) = 0;

    virtual void reboot(void) = 0;

    virtual void enableChecksum(bool checksumEnabled) = 0;

    virtual bool checksumEnabled(void) const = 0;

    virtual void flash(FirmwareImage* image) = 0;

    virtual FirmwareImage* image(void) const = 0;

signals:
    void errorMessageReceived(QString const& msg);

    void statusMessageReceived(QString const& msg);

    void deviceNotFound(void);

    void flasherProgressChanged(uint value);

    void flashingCompleted(void);

protected:
    explicit FirmwareUpgrader(QObject* parent = NULL);

};

#endif // FIRMWAREUPGRADERINTERFACE_H
