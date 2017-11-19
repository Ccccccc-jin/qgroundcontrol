#ifndef FLASHERPARAMETERS_H
#define FLASHERPARAMETERS_H

#include <QtCore>
#include "FirmwareVersion.h"

class FirmwareImage
{
public:
    enum Type {
        IMG, UNDEFINED
    };

    FirmwareImage(QString filename = "",
                  FirmwareVersion const& version = FirmwareVersion(),
                  Type type = Type::UNDEFINED)
        : _version(version), _filename(filename), _type(type) { }

    bool correct(void) const {
        auto info = QFileInfo(_filename);
        return info.exists() && info.isFile();
    }

    FirmwareVersion const& version (void) const { return _version; }
    QString         const& filename(void) const { return _filename; }

    Type type(void) const { return _type; }

private:
    FirmwareVersion _version;
    QString _filename;
    Type    _type;
};


class FlasherParameters
{
public:
    explicit FlasherParameters(FirmwareImage const& img = FirmwareImage(), bool checksumEnabled = true)
        : _fwImage(img), _checksumEnabled(checksumEnabled) { }

    bool checksumEnabled(void) const { return _checksumEnabled; }
    void enableChecksum(bool value) { _checksumEnabled = value; }

    FirmwareImage const& image(void) const { return _fwImage; }
    void setImage(FirmwareImage const& image) { _fwImage = image; }

private:
    FirmwareImage _fwImage;
    bool _checksumEnabled;
};

#endif // FLASHERPARAMETERS_H
