#ifndef EDGE_FIRMWARE_UPGRADER
#define EDGE_FIRMWARE_UPGRADER

#include <QtCore>

#include "firmware/FirmwareUpgraderInterface.h"
#include "EdgeMessageHandler.h"

class EdgeFirmwareUpgraderPrivate;

class EdgeFirmwareUpgrader : public FirmwareUpgrader
{
    Q_OBJECT
public:
    explicit EdgeFirmwareUpgrader(QObject* parent = nullptr);

    ~EdgeFirmwareUpgrader(void);

    void start(void) override final;

    void cancel(void) override final;

    void enableChecksum(bool checksumEnabled) override final;

    bool checksumEnabled(void) const override final;

    bool deviceAvailable(void) const override final;

    FirmwareInfo firmwareInfo(void) const override final;

    void flash(FirmwareImage const& image) override final;

signals:
    void _runRpiboot(void);
    void _runDeviceScanner(void);
    void _flash(QString firmwareImage, bool checksumEnabled);
    void _setFilterParams(int vid, QList<int> pids);

private slots:
    void _onRpiBootFinished        (bool status);
    void _onDeviceScanningFinished (bool status);
    void _onFlashingFinished       (bool status);

    void _onProcessErrorOcurred(QProcess::ProcessError err);
    void _onProcessStateChanged(QProcess::ProcessState state);

    void _runProcess(void);
    void _connectToProcess(void);
    void _connectToProcessSignals(void);

private:
    EdgeMessageHandler   _messageHandler;
    EdgeFirmwareUpgraderPrivate* _pimpl;
};

#endif //EDGE_FIRMWARE_UPGRADER
