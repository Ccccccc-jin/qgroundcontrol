#include "FirmwareVersion.h"

FirmwareVersion::FirmwareVersion(void)
    : _defined(false),
      _releaseDate(""),
      _major(0), _minor(0),
      _build(0), _rev(0)
{  }


FirmwareVersion::FirmwareVersion(uint maj, uint min, uint build, uint rev)
    : _defined(true),
      _releaseDate(""),
      _major(maj), _minor(min),
      _build(build), _rev(rev)
{  }


FirmwareVersion::FirmwareVersion(QString releaseDate, uint maj, uint min, uint build, uint rev)
    : _defined(true),
      _releaseDate(releaseDate),
      _major(maj), _minor(min),
      _build(build), _rev(rev)
{  }


bool FirmwareVersion::operator ==(FirmwareVersion const& version) const {
    return version._major == _major
        && version._minor == _minor
        && version._build == _build
        && version._rev   == _rev;
}


bool FirmwareVersion::operator !=(FirmwareVersion const& version) const
{
    return !(this->operator ==(version));
}


bool FirmwareVersion::operator >(FirmwareVersion const& version) const {
    return _major > version._major
        || _minor > version._minor
        || _build > version._build
        || _rev   > version._rev;
}


bool FirmwareVersion::operator >=(FirmwareVersion const& version) const
{
    return this->operator ==(version) || this->operator >(version);
}


bool FirmwareVersion::operator <(FirmwareVersion const& version) const
{
    return !(this->operator >=(version));
}


bool FirmwareVersion::operator <=(FirmwareVersion const& version) const
{
    return !(this->operator >(version));
}


QString FirmwareVersion::toString(int count) const {
    if (!defined() || (_major == 0 && _minor == 0 && _build == 0 && _rev == 0)) {
        return "undefined";
    }

    switch (count) {
        case 1: return QString::number(_major);
        case 2: return QString("%1.%2").arg(_major).arg(_minor);
        case 3: return QString("%1.%2.%3").arg(_major).arg(_minor).arg(_build);
        case 4: return QString("%1.%2.%3.%4").arg(_major).arg(_minor).arg(_build).arg(_rev);
        default: return QString("");
    }
}


FirmwareVersion FirmwareVersion::fromString(QString const& version, QString date)
{
    QString pattern = "(\\d)(\\.?)";
    QRegExp regex(pattern);
    FirmwareVersion fwversion;

    if (version.isEmpty()) {
        return fwversion;
    }

    fwversion.setReleaseDate(date);

    auto pos = 0;
    auto i = 0;

    int buf[4] = {0};

    while ((pos = regex.indexIn(version, pos)) != -1 && i < 3) {
        buf[i] = regex.cap(1).toUInt();
        pos += regex.matchedLength();
        i++;
    }

    fwversion.setMajor(buf[0]);
    fwversion.setMinor(buf[1]);
    fwversion.setBuild(buf[2]);
    fwversion.setRevision(buf[3]);

    fwversion.setDefined(true);

    return fwversion;
}
