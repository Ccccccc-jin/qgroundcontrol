#ifndef FIRMWAREUPGRADERCONTROLLER_H
#define FIRMWAREUPGRADERCONTROLLER_H

#include <QObject>

class FirmwareUpgraderController : public QObject
{
    Q_OBJECT
public:
    explicit FirmwareUpgraderController(QObject *parent = nullptr);

signals:

public slots:
};

#endif // FIRMWAREUPGRADERCONTROLLER_H