/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/


#ifndef FirmwareImage_H
#define FirmwareImage_H

#include <QtCore>

class FirmwareImage
{
public:
    enum class Type {
        IMG, PX4
    };

    FirmwareImage(QString const& filename,
                  QString const& version,
                  Type type = Type::IMG);
    
    QString const& filename(void) const { return _filename; }

    quint64 size(void) const { return _fileSize; }

    Type imageType(void) const { return _imageType; }

    QString version(void) const { return _version; }

    bool isValid(void) const;

private:
    QString _version;
    QString _filename;
    Type    _imageType;
    quint64 _fileSize;
};

#endif
