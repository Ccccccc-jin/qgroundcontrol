#ifndef FIRMWAREUPGRADERINTERFACE_H
#define FIRMWAREUPGRADERINTERFACE_H

#include <QObject>
#include <FlasherParameters.h>
#include <memory>

class FirmwareUpgrader : public QObject
{
    Q_OBJECT
public:
    static std::unique_ptr<FirmwareUpgrader> instance(void);
    virtual ~FirmwareUpgrader(void);

    virtual bool deviceAvailable(void) const = 0;
    virtual void flash(FlasherParameters const& image) = 0;

signals:
    void initialzed(void);
    void ready     (void);
    void finished  (void);
    void cancelled (void);

    void flashingFinished(bool status);

    void progressChanged(uint value);
    void firmwareVersionAvailable(QString const& version);

    void errorMessageReceived (QString const& msg);
    void infoMessageReceived  (QString const& msg);
    void warnMessageReceived  (QString const& msg);

public slots:
    virtual void start(void)  = 0;
    virtual void cancel(void) = 0;
    virtual void finish(void) = 0;

protected:
    explicit FirmwareUpgrader(QObject* parent = NULL);

};

#endif // FIRMWAREUPGRADERINTERFACE_H
