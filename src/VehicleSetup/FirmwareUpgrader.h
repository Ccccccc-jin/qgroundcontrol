#ifndef FIRMWAREUPGRADER_H
#define FIRMWAREUPGRADER_H

#include <QObject>

class FirmwareUpgrader : public QObject
{
    Q_OBJECT
public:
    explicit FirmwareUpgrader(QObject *parent = nullptr);

signals:

public slots:
};

#endif // FIRMWAREUPGRADER_H