/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/


/// @file
///     @author Don Gagne <don@thegagnes.com>

#ifndef FirmwareUpgradeController_H
#define FirmwareUpgradeController_H

#include <QTimer>

#include <stdint.h>
#include <memory>
#include "DeviceObserver.h"


class FirmwareUpgrader;

class FirmwareUpgradeController : public QObject
{
    Q_OBJECT
public:
    explicit FirmwareUpgradeController(void);
    ~FirmwareUpgradeController();

    Q_PROPERTY(bool    checksumEnabled  READ checksumEnabled  WRITE  enableChecksum)
    Q_PROPERTY(QString firmwareVersion  READ firmwareVersion  NOTIFY firmwareVersionAvailable)
    Q_PROPERTY(QString firmwareFilename READ firmwareFilename WRITE  setFirmwareFilename)

    Q_INVOKABLE void askForFirmwareFile(void);

    bool checksumEnabled(void) const;
    void enableChecksum(bool checksumEnabled);

    QString const& firmwareVersion(void)  const { return _firmwareVersion; }
    QString const& firmwareFilename(void) const { return _firmwareFilename; }

    void setFirmwareFilename(QString const& firmwareFilename)
        { _firmwareFilename = firmwareFilename; }

public slots:
    void observeDevice    (void);
    void initializeDevice (void);
    void flash            (void);
    void cancel           (void);

signals:
    void deviceInitializationStarted (void);
    void deviceFlashingStarted       (void);

    void devicePlugged     (void);
    void deviceUnplugged   (void);
    void cancelled         (void);
    void deviceInitialized (bool status);
    void deviceFlashed     (bool status);

    void flasherProgressChanged   (uint progress);
    void firmwareVersionAvailable (QString const& verison);

    void infoMsgReceived  (QString const& message);
    void errorMsgReceived (QString const& message);
    void warnMsgReceived  (QString const& message);

private:
    bool _deviceAvailable (void);
    void _startPolling    (void);
    void _initConnections (void);

    QString _firmwareFilename;
    QString _firmwareVersion;
    bool    _checksumEnabled;

    DeviceObserver                    _deviceObserver;
    std::unique_ptr<FirmwareUpgrader> _fwUpgrader;
};

#endif
