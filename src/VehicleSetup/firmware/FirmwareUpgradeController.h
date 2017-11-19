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

    explicit FirmwareUpgradeController(void);
    ~FirmwareUpgradeController();

    Q_INVOKABLE bool deviceAvailable(void) const;

    Q_INVOKABLE void flash(QString const& firmwareFilename);
    Q_INVOKABLE void askForFirmwareFile(void);

    bool checksumEnabled(void) const;
    void enableChecksum(bool checksumEnabled);

public slots:
    void searchDevice(void);
    void cancelFlashing(void);

signals:
    void deviceFound(void);
    void deviceNotFound(void);

    void flasherProgressChanged(uint progress);

    void infoMsgReceived  (QString const& message);
    void errorMsgReceived (QString const& message);

private slots:
    void _onTimeout(void);

private:
    void _startPolling(void);
    void _connectToFirmwareUpgrader(void);

    std::unique_ptr<FirmwareUpgrader> _fwUpgrader;
    QTimer _pollingTimer;
};

#endif
