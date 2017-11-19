#ifndef FIRMWAREVERSION_H
#define FIRMWAREVERSION_H

#include <QtCore>

struct FirmwareVersion {
    explicit FirmwareVersion(uint maj = 0, uint min = 0, uint build = 0, uint rev= 0)
        : _major(maj), _minor(min), _build(build), _rev(rev)
    {  }

    bool operator ==(FirmwareVersion const& version) const {
        return version._major == _major
            && version._minor == _minor
            && version._build == _build
            && version._rev   == _rev;
    }

    bool operator !=(FirmwareVersion const& version) const {
        return !(this->operator ==(version));
    }

    bool operator >(FirmwareVersion const& version) const {
        return _major > version._major
            || _minor > version._minor
            || _build > version._build
            || _rev   > version._rev;
    }

    bool operator >=(FirmwareVersion const& version) const{
        return this->operator ==(version) || this->operator >(version);
    }

    bool operator <(FirmwareVersion const& version) const{
        return !(this->operator >=(version));
    }

    bool operator <=(FirmwareVersion const& version) const{
        return !(this->operator >(version));
    }

    QString toString(int count = 2) const {
        switch (count) {
            case 1: return QString::number(_major);
            case 2: return QString("%1.%2").arg(_major).arg(_minor);
            case 3: return QString("%1.%2.%3").arg(_major).arg(_minor).arg(_build);
            case 4: return QString("%1.%2.%3.%4").arg(_major).arg(_minor).arg(_build).arg(_rev);
            default: return QString("");
        }
    }

    static FirmwareVersion fromString(QString const& version) {
        QString pattern = "(\\d)(\\.?)";
        QRegExp regex(pattern);
        FirmwareVersion fwversion;

        auto pos = 0;
        auto i = 0;

        int buf[4] = {0};

        while ((pos = regex.indexIn(version, pos)) != -1 && i < 3) {
            qInfo() << pos;
            buf[i] = regex.cap(1).toUInt();
            pos += regex.matchedLength();
            i++;
        }

        fwversion.setMajor(buf[0]);
        fwversion.setMinor(buf[1]);
        fwversion.setBuild(buf[2]);
        fwversion.setRevision(buf[3]);

        return fwversion;
    }

    uint getMajor   (void) const { return _major; }
    uint getMinor   (void) const { return _minor; }
    uint getBuild   (void) const { return _build; }
    uint getRevision(void) const { return _rev; }

    void setMajor   (uint value) { _major = value; }
    void setMinor   (uint value) { _minor = value; }
    void setBuild   (uint value) { _build = value; }
    void setRevision(uint value) { _rev   = value; }

private:
    uint _major;
    uint _minor;
    uint _build;
    uint _rev;
};

#endif // FIRMWAREVERSION_H
