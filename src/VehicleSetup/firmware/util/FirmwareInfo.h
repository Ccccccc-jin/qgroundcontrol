#ifndef FIRMWAREINFO_H
#define FIRMWAREINFO_H

#include "FirmwareVersion.h"

class EdgeRemoteFirmwareInfo
{
public:
    static QString firmwareUrl(void) {
        return "http://files.emlid.com/edge/firmware";
    }

    static QString firmwareInfoFileUrl(void) {
        return "http://files.emlid.com/edge/firmware/info.json";
    }

    EdgeRemoteFirmwareInfo(void) : _isUndefined(true) {}

    EdgeRemoteFirmwareInfo(FirmwareVersion const& version,
                       qint64 archiveSize,
                       qint64 imageSize,
                       QString const& firmwareArchiveName);


    qint64 archiveSize(void) const { return _archiveSize; }
    qint64 imageSize(void)   const { return _imageSize; }

    FirmwareVersion const& version(void)             const { return _version; }
    QString         const& firmwareArchiveName(void) const { return _firmwareImageName; }
    QString         const& archiveUrl(void)          const { return _archiveUrl; }

    FirmwareVersion version(void)             { return _version; }
    QString         firmwareArchiveName(void) { return _firmwareImageName; }
    QString         archiveUrl(void)          { return _archiveUrl; }

    bool isUndefined(void) const { return _isUndefined; }
    void fromJson(QByteArray const& jsonData);

private:
    bool            _isUndefined;
    FirmwareVersion _version;
    qint64          _archiveSize;
    qint64          _imageSize;

    QString         _firmwareImageName;
    QString         _firmwareImageUrl;
    QString         _archiveUrl;
};

#endif // FIRMWAREINFO_H
