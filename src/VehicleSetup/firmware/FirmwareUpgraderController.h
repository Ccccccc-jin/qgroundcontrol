#ifndef FIRMWAREUPGRADERCONTROLLER_H
#define FIRMWAREUPGRADERCONTROLLER_H

#include "PX4FirmwareUpgradeThread.h"
#include "LinkManager.h"
#include "FirmwareImage.h"

#include <QObject>
#include <QUrl>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPixmap>
#include <QQuickItem>

#include "qextserialport.h"

#include <stdint.h>

class FirmwareUpgraderController : public QObject
{
    Q_OBJECT
public:
    FirmwareUpgraderController(void);
    ~FirmwareUpgraderController();

    Q_PROPERTY(QString          boardPort                   READ boardPort                                              NOTIFY boardFound)
    Q_PROPERTY(QString          boardDescription            READ boardDescription                                       NOTIFY boardFound)

    Q_PROPERTY(QQuickItem* statusLog READ statusLog WRITE setStatusLog)
    Q_PROPERTY(QQuickItem* progressBar READ progressBar WRITE setProgressBar)

    Q_INVOKABLE void startBoardSearch(void);

    Q_INVOKABLE void cancel(void);

    Q_INVOKABLE void flash(AutoPilotStackType_t stackType,
                           FirmwareType_t firmwareType = StableFirmware,
                           FirmwareVehicleType_t vehicleType = DefaultVehicleFirmware );

    // Property accessors

    QQuickItem* progressBar(void) { return _progressBar; }
    void setProgressBar(QQuickItem* progressBar) { _progressBar = progressBar; }

    QQuickItem* statusLog(void) { return _statusLog; }
    void setStatusLog(QQuickItem* statusLog) { _statusLog = statusLog; }

    QString boardDescription(void) { return _foundBoardInfo.description(); }

signals:
    void boardFound(void);
    void noBoardFound(void);
    void boardGone(void);
    void flashComplete(void);
    void flashCancelled(void);
    void error(void);

private:
    QQuickItem*     _statusLog;         ///< Status log TextArea Qml control
    QQuickItem*     _progressBar;
};

#endif // FIRMWAREUPGRADERCONTROLLER_H
