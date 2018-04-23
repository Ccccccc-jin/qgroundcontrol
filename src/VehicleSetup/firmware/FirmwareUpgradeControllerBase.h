#ifndef FIRMWAREUPGRADECONTROLLERBASE_H
#define FIRMWAREUPGRADECONTROLLERBASE_H

#include <QtCore>
#include "RemoteFirmwareInfoViewBase.h"
#include "FirmwareUpdateSettings.h"
#include "util/FirmwareVersion.h"

class FirmwareUpgradeControllerBase : public QObject
{
    Q_OBJECT
public:
    virtual ~FirmwareUpgradeControllerBase() {}

    enum UpdateMethod {
        Auto, Manual
    }; Q_ENUM(UpdateMethod)

    Q_PROPERTY(QString                     firmwareVersion       READ firmwareVersion       NOTIFY firmwareVersionAvailable)
    Q_PROPERTY(QString                     firmwareFilename      READ firmwareFilename      WRITE  setFirmwareFilename)
    Q_PROPERTY(bool                        checksumEnabled       READ checksumEnabled       WRITE  enableChecksum)
    Q_PROPERTY(UpdateMethod                updateMethod          READ updateMethod          WRITE  setUpdateMethod NOTIFY updateMethodChanged)
    Q_PROPERTY(RemoteFirmwareInfoViewBase* remoteFirmwareInfo    READ remoteFirmwareInfo    NOTIFY remoteFirmwareInfoChanged)
    Q_PROPERTY(QString                     availableDiskSpace    READ availableDiskSpace    NOTIFY availableDiskSpaceChanged)
    Q_PROPERTY(bool                        firmwareSavingEnabled READ firmwareSavingEnabled WRITE  enableFirmwareSaving)

    Q_INVOKABLE virtual void askForFirmwareFile(void) = 0;
    Q_INVOKABLE virtual void askForFirmwareDirectory(void) = 0;
    Q_INVOKABLE virtual bool hasEnoughDiskSpace(void) = 0;

    virtual RemoteFirmwareInfoViewBase* remoteFirmwareInfo(void) = 0;
    virtual QString availableDiskSpace(void) const = 0;

    QString      firmwareFilename      (void) const { return _firmwareFilename; }
    QString      firmwareVersion       (void) const { return _firmwareVersion.toString(); }
    UpdateMethod updateMethod          (void) const { return _updateMethod; }
    bool         checksumEnabled       (void) const { return _settings.checksumEnabeld(); }
    bool         firmwareSavingEnabled (void) const { return _firmwareSavingEnabled; }

    void enableChecksum       (bool checksumEnabled)             { _settings.setChecksumEnabled(checksumEnabled); }
    void enableFirmwareSaving (bool enableSaving)                { _firmwareSavingEnabled = enableSaving; }
    void setUpdateMethod      (UpdateMethod updateMethod)        { _updateMethod = updateMethod; emit updateMethodChanged(updateMethod); }
    void setFirmwareFilename  (QString const& firmwareFilename)  { _firmwareFilename = firmwareFilename; }

public slots:
    virtual void observeDevice    (void) = 0;
    virtual void initializeDevice (void) = 0;
    virtual void flash            (void) = 0;
    virtual void cancel           (void) = 0;

signals:
    void updateMethodChanged            (UpdateMethod method);
    void firmwareVersionAvailable       (QString const& verison);
    void remoteFirmwareVersionAvailable (QString const& verison);

    void remoteFirmwareInfoChanged(void);
    void availableDiskSpaceChanged(void);

    void deviceInitializationStarted (void);
    void firmwareUpgraderStarted     (void);

    void devicePlugged   (void);
    void deviceUnplugged (void);

    void cancelled         (void);
    void deviceInitialized (bool status);
    void deviceFlashed     (bool status);
    void connectionWithUpdaterError(void);

    void flasherProgressChanged(uint progress);

    void infoMsgReceived  (QString const& message);
    void errorMsgReceived (QString const& message);
    void warnMsgReceived  (QString const& message);
    void firmwareInfoMsg  (QString const& message);

protected:
    explicit FirmwareUpgradeControllerBase(QObject* parent = nullptr)
        : QObject(parent),
          _firmwareSavingEnabled(false),
          _updateMethod(UpdateMethod::Auto)
    { }

    FirmwareVersion        _firmwareVersion;
    FirmwareUpdateSettings _settings;

    bool         _firmwareSavingEnabled;
    QString      _firmwareFilename;
    UpdateMethod _updateMethod;
};

#endif // FIRMWAREUPGRADECONTROLLERBASE_H
