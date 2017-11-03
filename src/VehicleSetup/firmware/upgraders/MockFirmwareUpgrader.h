#ifndef MOCKFIRMWAREUPGRADER_H
#define MOCKFIRMWAREUPGRADER_H

#include <QtCore>
#include "firmware/FirmwareUpgraderInterface.h"


class MockFirmwareUpgrader : public FirmwareUpgrader
{
    Q_OBJECT
public:
    MockFirmwareUpgrader(QObject* parent = nullptr);

    ~MockFirmwareUpgrader(void) override final;

    void cancel(void) override final;

    void reboot(void) override final;

    void flash(FirmwareImage const* image) override final;

    FirmwareImage* image(void) const override final;

private:
    QTimer _timer;
    uint _progress;
};

#endif // MOCKFIRMWAREUPGRADER_H
