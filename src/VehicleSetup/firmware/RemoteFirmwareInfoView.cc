#include "RemoteFirmwareInfoView.h"

RemoteFirmwareInfoView::RemoteFirmwareInfoView(QObject *parent)
    : RemoteFirmwareInfoViewBase(parent)
{ }


QString RemoteFirmwareInfoView::imageSize(void) const
{
    return _remoteFirmwareInfo.isUndefined() ?
                "undefined" : toMBytes(_remoteFirmwareInfo.imageSize());
}


QString RemoteFirmwareInfoView::version(void) const
{
    return _remoteFirmwareInfo.isUndefined() ?
                "undefined" : _remoteFirmwareInfo.version().toString();
}


QString RemoteFirmwareInfoView::diskSpace(void) const
{
    return _remoteFirmwareInfo.isUndefined() ?
                "undefined" : toMBytes(_remoteFirmwareInfo.archiveSize() + _remoteFirmwareInfo.imageSize());
}


QString RemoteFirmwareInfoView::releaseDate(void) const
{
    return _remoteFirmwareInfo.isUndefined() ?
                "undefined" : _remoteFirmwareInfo.version().releaseDate();
}

void RemoteFirmwareInfoView::
    setRemoteFirmwareInfo(const EdgeRemoteFirmwareInfo &firmwareInfo)
{
    _remoteFirmwareInfo = firmwareInfo;
    emit infoViewChanged();
}


QString RemoteFirmwareInfoView::toMBytes(qint64 bytes) const
{
    return QString("%1 Mb").arg(bytes / (1024LL * 1024LL));
}
