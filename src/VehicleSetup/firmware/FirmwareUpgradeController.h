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
#include "DeviceObserver.h"
#include "QGCFileDownload.h"
#include "QGCXzDecompressor.h"
#include "FirmwareVersion.h"

#include "RemoteFirmwareManager.h"
#include "RemoteFirmwareInfoView.h"

class FirmwareUpgrader;

class FirmwareUpgradeController : public FirmwareUpgradeControllerBase
{
    Q_OBJECT
public:
    explicit FirmwareUpgradeController(void);
    ~FirmwareUpgradeController();

    Q_INVOKABLE virtual void askForFirmwareFile(void) override;
    Q_INVOKABLE virtual void askForFirmwareDirectory(void) override;
    Q_INVOKABLE virtual bool hasEnoughDiskSpace(void) override;

    virtual RemoteFirmwareInfoViewBase* remoteFirmwareInfo(void) override { return _remoteFirmwareInfoView.get(); }

    virtual QString      availableDiskSpace    (void) const override { return QString("%1 Mb").arg(_availableDiskSpace / 1024 / 1024); }
    virtual QString      firmwareFilename      (void) const override { return _firmwareFilename;  }
    virtual QString      firmwareVersion       (void) const override { return _firmwareVersion.toString(); }
    virtual UpdateMethod updateMethod          (void) const override { return _updateMethod; }
    virtual bool         checksumEnabled       (void) const override { return _checksumEnabled; }
    virtual bool         firmwareSavingEnabled (void) const override { return _firmwareSavingEnabled; }

    virtual void enableChecksum(bool value) override
        { _checksumEnabled = value; }

    virtual void enableFirmwareSaving(bool value) override
        { _firmwareSavingEnabled = value; }

    virtual void setUpdateMethod(UpdateMethod value) override
        { _updateMethod = value; emit updateMethodChanged(_updateMethod);}

    virtual void setFirmwareFilename(QString const& firmwareFilename) override
        { _firmwareFilename = firmwareFilename; }


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
    void _initConnections        (void);
    void _attachFirmwareUpgrader (void);
    void _attachDeviceObserver   (void);
    void _fetchFirmwareInfo  (void);
    void _attachFirmwareManager  (void);

    void _removeDownloadedFiles  (void);

    void _flashSelectedFile(void);
    bool _deviceAvailable  (void);
    void _startPolling     (void);

    FirmwareVersion _firmwareVersion;
    QString _firmwareDirectory;
    QString _firmwareFilename;
    bool    _checksumEnabled;
    bool    _firmwareSavingEnabled;
    qint64  _availableDiskSpace;

    RemoteFirmwareManager                   _remoteFwManager;
    UpdateMethod                            _updateMethod;
    DeviceObserver                          _deviceObserver;
    std::unique_ptr<QGCDownloadWatcher>     _downloadWatcher;
    std::unique_ptr<RemoteFirmwareInfoView> _remoteFirmwareInfoView;
    std::unique_ptr<FirmwareUpgrader>       _fwUpgrader;
};

#endif
