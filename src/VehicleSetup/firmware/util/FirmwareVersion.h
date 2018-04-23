#ifndef FIRMWAREVERSION_H
#define FIRMWAREVERSION_H

#include <QtCore>


struct FirmwareVersion {
    explicit FirmwareVersion(void);
    explicit FirmwareVersion(uint maj, uint min = 0, uint build = 0, uint rev= 0);
    explicit FirmwareVersion(QString releaseDate, uint maj = 0, uint min = 0,
                             uint build = 0, uint rev= 0);

    bool operator ==(FirmwareVersion const& version) const;
    bool operator !=(FirmwareVersion const& version) const;
    bool operator > (FirmwareVersion const& version) const;
    bool operator >=(FirmwareVersion const& version) const;
    bool operator < (FirmwareVersion const& version) const;
    bool operator <=(FirmwareVersion const& version) const;

    QString toString(int count = 2) const;

    static FirmwareVersion fromString(QString const& version, QString date = "");

    uint getMajor   (void) const { return _major; }
    uint getMinor   (void) const { return _minor; }
    uint getBuild   (void) const { return _build; }
    uint getRevision(void) const { return _rev; }

    void setMajor   (uint value) { _major = value; }
    void setMinor   (uint value) { _minor = value; }
    void setBuild   (uint value) { _build = value; }
    void setRevision(uint value) { _rev   = value; }

    QString const& releaseDate(void) const { return _releaseDate; }
    void setReleaseDate(QString const& releaseDate) { _releaseDate = releaseDate; }

    bool defined(void) const { return _defined; }
    void setDefined(bool defined) { _defined = defined; }

private:
    bool _defined;
    QString _releaseDate;
    uint _major;
    uint _minor;
    uint _build;
    uint _rev;
};

#endif // FIRMWAREVERSION_H
