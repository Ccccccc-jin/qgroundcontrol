#ifndef REMOTEFIRMWAREINFOVIEWBASE_H
#define REMOTEFIRMWAREINFOVIEWBASE_H

#include <QObject>

class RemoteFirmwareInfoViewBase : public QObject
{
    Q_OBJECT
public:
    virtual ~RemoteFirmwareInfoViewBase(void) { }

    Q_PROPERTY(QString imageSize    READ imageSize    NOTIFY infoViewChanged)
    Q_PROPERTY(QString version      READ version      NOTIFY infoViewChanged)
    Q_PROPERTY(QString diskSpace    READ diskSpace    NOTIFY infoViewChanged)
    Q_PROPERTY(QString releaseDate  READ releaseDate  NOTIFY infoViewChanged)

    virtual QString imageSize   (void) const { return ""; }
    virtual QString version     (void) const { return ""; }
    virtual QString diskSpace   (void) const { return ""; }
    virtual QString releaseDate (void) const { return ""; }

signals:
    void infoViewChanged(void);

protected:
    explicit RemoteFirmwareInfoViewBase(QObject *parent = nullptr)
        : QObject(parent)
    { }
};

#endif // REMOTEFIRMWAREINFOVIEWBASE_H
