#ifndef EDGE_FIRMWARE_UPGRADER
#define EDGE_FIRMWARE_UPGRADER

#include <QtCore>

#include "firmware/FirmwareUpgraderInterface.h"


class EdgeFirmwareUpgraderPrivate;


class EdgeFirmwareUpgrader : public FirmwareUpgrader
{
    Q_OBJECT
public:
    explicit EdgeFirmwareUpgrader(QObject* parent = nullptr);

    ~EdgeFirmwareUpgrader(void);

    bool deviceAvailable(void) const override final;

    void cancel(void) override final;

    void reboot(void) override final;

    void enableChecksum(bool checksumEnabled) override final;

    bool checksumEnabled(void) const override final;

    FirmwareImage* image(void) const override final;

    void flash(FirmwareImage* image) override final;

signals:
    void start(QString firmwareImage, bool checksumEnabled);

private slots:
    void _onWatcherInitialized(void);

    void _onWatcherSubsystemStateChanged(QString subsystem, uint state);

    void _fwUpgraderProcessErrrorOcurred(QProcess::ProcessError err);

    void _onProcessStateChanged(QProcess::ProcessState state);

private:
    void _sendStatusMessage(QString const& message);

    EdgeFirmwareUpgraderPrivate* _pimpl;
};

#endif //EDGE_FIRMWARE_UPGRADER
