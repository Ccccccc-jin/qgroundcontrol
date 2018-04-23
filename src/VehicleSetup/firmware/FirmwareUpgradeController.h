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
#include "FirmwareUpgradeControllerBase.h"

#include <stdint.h>
#include <memory>
#include "UsbPluginNotifier.h"
#include "QGCFileDownload.h"
#include "QGCXzDecompressor.h"

#include "util/FirmwareVersion.h"
#include "util/RemoteFirmwareManager.h"

#include "RemoteFirmwareInfoView.h"
#include "FirmwareUpdateSettings.h"

namespace client {
    class UpdaterConnection;
}

class FirmwareUpgradeController : public FirmwareUpgradeControllerBase
{
    Q_OBJECT
public:
    explicit FirmwareUpgradeController(void);
    ~FirmwareUpgradeController(void) override;

    Q_INVOKABLE virtual void askForFirmwareFile(void) override;
    Q_INVOKABLE virtual void askForFirmwareDirectory(void) override;
    Q_INVOKABLE virtual bool hasEnoughDiskSpace(void) override;

    virtual RemoteFirmwareInfoViewBase* remoteFirmwareInfo(void) override
        { return _remoteFirmwareInfoView.get(); }

    QString availableDiskSpace(void) const override;

public slots:
    void observeDevice    (void) override;
    void initializeDevice (void) override;
    void flash            (void) override;
    void cancel           (void) override;

signals:
    void _cancel           (void);
    void _flash            (void);

private slots:
    void _onNetworkError(QNetworkReply::NetworkError);
    void _onDecompressError(QGCXzDecompressor::ErrorType);
    void _onCancelled(void);

private:
    // Methods for connect signals/slots
    qint64 _availableDiskSpace   (QString const& storage) const;
    void _initConnections        (void);
    void _attachFirmwareUpdater (void);
    void _attachConnection       (void);
    void _attachPlugInNotifier   (void);
    void _fetchFirmwareInfo      (void);
    void _attachFirmwareManager  (void);

    void _removeDownloadedFiles  (void);

    void _flashSelectedFile (void);
    bool _deviceAvailable   (void);
    void _startPolling      (void);
    void _initializeDevice  (void);

    bool                                    _updaterAttached;
    RemoteFirmwareManager                   _remoteFwManager;
    UsbPluginNotifier                       _pluginNotifier;

    std::unique_ptr<client::UpdaterConnection>      _connection;
    std::unique_ptr<QGCDownloadWatcher>     _downloadWatcher;
    std::unique_ptr<RemoteFirmwareInfoView> _remoteFirmwareInfoView;
};

#endif
