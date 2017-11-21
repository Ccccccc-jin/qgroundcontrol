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

    Q_PROPERTY(bool    checksumEnabled  READ checksumEnabled  WRITE enableChecksum)
    Q_PROPERTY(QString firmwareVersion  READ firmwareVersion  NOTIFY firmwareVersionAvailable)
    Q_PROPERTY(QString firmwareFilename READ firmwareFilename WRITE setFirmwareFilename)

    Q_INVOKABLE void start(void);
    Q_INVOKABLE void flash(void);

    Q_INVOKABLE bool deviceAvailable(void) const;
    Q_INVOKABLE void askForFirmwareFile(void);

    bool checksumEnabled(void) const;
    void enableChecksum(bool checksumEnabled);

    QString const& firmwareVersion(void)  const { return _firmwareVersion; }
    QString const& firmwareFilename(void) const { return _firmwareFilename; }
    void setFirmwareFilename(QString const& firmwareFilename) { _firmwareFilename = firmwareFilename; }

public slots:
    void searchDevice (void);
    void cancel       (void);

signals:
    void devicePlugged   (void);
    void deviceUnplugged (void);

    void ready     (void);
    void finished  (void);
    void cancelled (void);

    void flashingFinished(bool status);
    void flasherProgressChanged(uint progress);
    void firmwareVersionAvailable(QString const& verison);

    void infoMsgReceived  (QString const& message);
    void errorMsgReceived (QString const& message);
    void warnMsgReceived  (QString const& message);

private:
    void _startPolling(void);
    void _initConnections(void);

    QString _firmwareFilename;
    QString _firmwareVersion;
    bool    _checksumEnabled;
    bool    _deviceBootAsMassStorage;

    DeviceObserver _deviceObserver;
    std::unique_ptr<FirmwareUpgrader> _fwUpgrader;
};

#endif
