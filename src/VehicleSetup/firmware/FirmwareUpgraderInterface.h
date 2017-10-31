#ifndef FIRMWAREUPGRADERINTERFACE_H
#define FIRMWAREUPGRADERINTERFACE_H

#include <QObject>
#include "FirmwareImage.h"

class FirmwareUpgrader : public QObject
{
    Q_OBJECT

public:
    virtual void cancel(void) = 0;

    virtual void reboot(void) = 0;

    virtual void flash(FirmwareImage const* image) = 0;

    virtual FirmwareImage* image(void) const = 0;

signals:
    void errorMessageReceived(QString const& msg);

    void statusMessageReceived(QString const& msg);

    void deviceNotFound(void);

    void flasherProgressChanged(uint value);

    void flashingCompleted(void);

protected:
    explicit FirmwareUpgrader(QObject* parent = NULL);

    virtual ~FirmwareUpgrader(void) = 0;
};

#endif // FIRMWAREUPGRADERINTERFACE_H
