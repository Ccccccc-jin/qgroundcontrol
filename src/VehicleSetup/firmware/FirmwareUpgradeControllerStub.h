#ifndef FIRMWAREUPGRADECONTROLLERSTUB_H
#define FIRMWAREUPGRADECONTROLLERSTUB_H

#include <QObject>
#include "FirmwareUpgradeControllerBase.h"

class FirmwareUpgradeControllerStub : public FirmwareUpgradeControllerBase
{
    Q_OBJECT
public:
    explicit FirmwareUpgradeControllerStub(QObject* parent = nullptr)
        : FirmwareUpgradeControllerBase(parent)
    {
        emit infoMsgReceived("Firmware updating not supported.");
    }

    ~FirmwareUpgradeControllerStub(void) {}

    Q_INVOKABLE virtual void askForFirmwareFile(void) override { }
    Q_INVOKABLE virtual void askForFirmwareDirectory(void) override { }
    Q_INVOKABLE virtual bool hasEnoughDiskSpace(void) override { return false; }

    virtual RemoteFirmwareInfoViewBase* remoteFirmwareInfo(void) override { return nullptr; }

    virtual QString      availableDiskSpace    (void) const override { return ""; }
    virtual QString      firmwareFilename      (void) const override { return ""; }
    virtual QString      firmwareVersion       (void) const override { return ""; }
    virtual UpdateMethod updateMethod          (void) const override { return UpdateMethod::Auto; }
    virtual bool         checksumEnabled       (void) const override { return false; }
    virtual bool         firmwareSavingEnabled (void) const override { return false; }

    virtual void enableChecksum(bool value)          override { Q_UNUSED(value); }
    virtual void enableFirmwareSaving(bool value)    override { Q_UNUSED(value); }
    virtual void setUpdateMethod(UpdateMethod value) override { Q_UNUSED(value); }
    virtual void setFirmwareFilename(QString const& firmwareFilename) override { Q_UNUSED(firmwareFilename); }

public slots:
    void observeDevice    (void) override { }
    void initializeDevice (void) override { }
    void flash            (void) override { }
    void cancel           (void) override { }

};

#endif // FIRMWAREUPGRADECONTROLLERSTUB_H
