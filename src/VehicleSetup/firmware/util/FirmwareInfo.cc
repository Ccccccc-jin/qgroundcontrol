#include "FirmwareInfo.h"


EdgeRemoteFirmwareInfo::EdgeRemoteFirmwareInfo(FirmwareVersion const& version,
                                       qint64 archiveSize,
                                       qint64 imageSize,
                                       QString const& firmwareUrl)
    : _isUndefined(false),
      _version(version),
      _archiveSize(archiveSize),
      _imageSize(imageSize),
      _firmwareImageName(firmwareUrl)
{ }


void EdgeRemoteFirmwareInfo::fromJson(QByteArray const& jsonData)
{
    auto jsonObject = QJsonDocument::fromJson(jsonData).object();

    auto firmwareVersion = jsonObject["version"].toString();
    auto releaseDate     = jsonObject["date"].toString();
    _imageSize   = static_cast<qint64>(jsonObject["imageSize"].toDouble());
    _archiveSize = static_cast<qint64>(jsonObject["archiveSize"].toDouble());
    _version = FirmwareVersion::fromString(firmwareVersion, releaseDate);

    _firmwareImageName = QString("%1-edge-emlid-v%2.img.xz")
            .arg(releaseDate).arg(_version.toString());

    _isUndefined = false;
}
