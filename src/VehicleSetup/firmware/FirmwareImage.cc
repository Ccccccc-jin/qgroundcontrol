/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/


/// @file
///     @brief Support for Intel Hex firmware file
///     @author Don Gagne <don@thegagnes.com>

#include "FirmwareImage.h"


FirmwareImage::FirmwareImage(QString const& filename,
                             QString const& version,
                             FirmwareImage::Type type)
   : _version(version),
     _filename(filename),
     _imageType(type),
     _fileSize(0)
{ }


bool FirmwareImage::isValid(void) const
{
    auto fileInfo = QFileInfo(_filename);

    return fileInfo.isFile();
}
