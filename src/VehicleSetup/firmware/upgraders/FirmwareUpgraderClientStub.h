#ifndef FIRMWAREUPGRADERCLIENTSTUB_H
#define FIRMWAREUPGRADERCLIENTSTUB_H


#include <QObject>
#include <memory>
#include "FlasherParameters.h"
#include "firmware/FirmwareUpgraderInterface.h"

class FirmwareUpgraderClientStub : public FirmwareUpgrader
{
    Q_OBJECT
public:
    explicit FirmwareUpgraderClientStub(QObject* parent = NULL)
        : _msgSent(false)
    {
        Q_UNUSED(parent);
        QObject::connect(this, &FirmwareUpgraderClientStub::_infoMsg,
                         this, &FirmwareUpgraderClientStub::infoMessageReceived);
    }

    virtual ~FirmwareUpgraderClientStub(void) { }

    virtual bool deviceAvailable(void) const {
        if (!_msgSent) {
            auto msg = "Firmware upgrading temporary unsupported "
                       "on this platform.";
            emit _infoMsg(msg);
            _msgSent = true;
        }
        return false;
    }

public slots:
    virtual void initializeDevice(void) { }
    virtual void flash(FlasherParameters const& image) { Q_UNUSED(image); }
    virtual void cancel(void) { }


signals:
    void _infoMsg(QString const& str) const;

private:
    mutable bool _msgSent;
};

#endif // FIRMWAREUPGRADERCLIENTSTUB_H
