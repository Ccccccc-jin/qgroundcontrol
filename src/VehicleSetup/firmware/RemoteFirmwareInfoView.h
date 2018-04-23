#ifndef REMOTEFIRMWAREINFOVIEW_H
#define REMOTEFIRMWAREINFOVIEW_H

#include <QObject>
#include "util/FirmwareInfo.h"
#include "RemoteFirmwareInfoViewBase.h"

class RemoteFirmwareInfoView : public RemoteFirmwareInfoViewBase
{
    Q_OBJECT
public:
    explicit RemoteFirmwareInfoView(QObject *parent = nullptr);

    virtual QString imageSize(void)   const override;
    virtual QString version(void)     const override;
    virtual QString diskSpace(void)   const override;
    virtual QString releaseDate(void) const override;

    void setRemoteFirmwareInfo(EdgeRemoteFirmwareInfo const& firmwareInfo);

    EdgeRemoteFirmwareInfo const& remoteFirmwareInfo(void) const { return _remoteFirmwareInfo; }
    EdgeRemoteFirmwareInfo remoteFirmwareInfo(void) { return _remoteFirmwareInfo; }

private:
    QString toMBytes(qint64 bytes) const;

    EdgeRemoteFirmwareInfo _remoteFirmwareInfo;
};

#endif // REMOTEFIRMWAREINFOVIEW_H
