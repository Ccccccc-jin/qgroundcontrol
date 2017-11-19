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


class FirmwareUpgrader;

class FirmwareUpgradeController : public QObject
{
    Q_OBJECT
public:
    Q_PROPERTY(bool checksumEnabled
               READ checksumEnabled WRITE enableChecksum)

    Q_PROPERTY(QString firmwareVersion
               READ firmwareVersion NOTIFY firmwareVersionAvailable)

    Q_PROPERTY(QString firmwareFilename
               READ firmwareFilename WRITE setFirmwareFilename)

    explicit FirmwareUpgradeController(void);
    ~FirmwareUpgradeController();

    Q_INVOKABLE bool deviceAvailable(void) const;

    Q_INVOKABLE void start(void);

    Q_INVOKABLE void flash(void);
    Q_INVOKABLE void askForFirmwareFile(void);

    QString firmwareVersion(void) { return _firmwareVersion; }

    bool checksumEnabled(void) const;
    void enableChecksum(bool checksumEnabled);

    QString const& firmwareFilename(void) const { return _firmwareFilename; }
    void setFirmwareFilename(QString const& firmwareFilename) { _firmwareFilename = firmwareFilename; }

public slots:
    void searchDevice(void);
    void cancel(void);

signals:
    void deviceFound(void);
    void deviceNotFound(void);
    void flashingFinished(bool status);

    void started();
    void finished();
    void cancelled();

    void firmwareVersionAvailable(QString const& verison);
    void flasherProgressChanged(uint progress);

    void infoMsgReceived  (QString const& message);
    void errorMsgReceived (QString const& message);
    void warnMsgReceived  (QString const& message);

private slots:
    void _onTimeout(void);

private:
    void _startPolling(void);
    void _connectToFirmwareUpgrader(void);

    QString _firmwareFilename;
    QString _firmwareVersion;
    bool _checksumEnabled;
    std::unique_ptr<FirmwareUpgrader> _fwUpgrader;
    QTimer _pollingTimer;
};

#endif
