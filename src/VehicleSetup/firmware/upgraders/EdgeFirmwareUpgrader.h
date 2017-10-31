#ifndef EDGE_FIRMWARE_UPGRADER
#define EDGE_FIRMWARE_UPGRADER

#include <QtCore>

#include "firmware/FirmwareUpgraderInterface.h"


class EdgeFirmwareUpgraderPrivate;


class EdgeFirmwareUpgrader : public FirmwareUpgraderInterface
{
    Q_OBJECT
public:
    explicit EdgeFirmwareUpgrader(QObject* parent = nullptr);

    ~EdgeFirmwareUpgrader(void);

    void cancel(void) override final;

    void reboot(void) override final;

    void flash(FirmwareImage const* image) override final;

    FirmwareImage* image(void) const override final;

signals:
    void start(void);

private slots:
    void _onWatcherInitialized(void);

    void _onWatcherSubsystemStateChanged(QString subsystem, uint state);

    void _fwUpgraderProcessErrrorOcurred(QProcess::ProcessError err);

private:
    void _sendStatusMessage(QString const& message);

    EdgeFirmwareUpgraderPrivate* _pimpl;
};

#endif //EDGE_FIRMWARE_UPGRADER
