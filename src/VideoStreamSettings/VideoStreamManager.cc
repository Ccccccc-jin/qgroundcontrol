/****************************************************************************
 *
 *   (c) 2009-2017 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include <QNetworkInterface>
#include <QHostInfo>

#include "VideoStreamManager.h"
#include "Vehicle.h"
#include "QGCMAVLink.h"
#include "QGCApplication.h"

#define CAMERA_ID 1

VideoStreamManager::VideoStreamManager(Vehicle* vehicle)
    : _vehicle(vehicle)
    , _mavlink(NULL)
    , _targetIp("None")
    , _targetPort(5600)
    , _videoStatusFact          (0, "videoStatus",          FactMetaData::valueTypeUint8)
    , _resolutionFact           (0, "resolution",           FactMetaData::valueTypeUint8)
    , _resolutionHorizontalFact (0, "resolutionHorizontal", FactMetaData::valueTypeUint16)
    , _resolutionVerticalFact   (0, "resolutionVertical",   FactMetaData::valueTypeUint16)
    , _bitRateFact              (0, "bitRate",              FactMetaData::valueTypeUint32)
    , _fpsFact                  (0, "fps",                  FactMetaData::valueTypeFloat)
    , _rotationFact             (0, "rotation",             FactMetaData::valueTypeUint16)
    , _resolutionMetaData (FactMetaData::valueTypeUint8)
    , _bitRateMetaData    (FactMetaData::valueTypeUint32)
    , _fpsMetaData        (FactMetaData::valueTypeFloat)
    , _rotationMetaData   (FactMetaData::valueTypeUint16)
{
    _mavlink = qgcApp()->toolbox()->mavlinkProtocol();

    QStringList  bitRateStrings, fpsStrings, rotationStrings;
    QVariantList resolutionValues, bitRateValues, fpsValues, rotationValues;

    _resolutionList << "640x480" << "720x576" << "1280x720" << "1920x1080";
    resolutionValues << 0 << 1 << 2 << 3;
    bitRateStrings << "Auto" << "1 Mb" << "2 Mb" << "3 Mb" << "5 Mb" << "10 Mb";
    bitRateValues << 0 << 1000000 << 2000000 << 3000000 << 5000000 << 10000000;
    fpsStrings << "Auto" << "15 Hz" << "30 Hz" << "60 Hz" << "90 Hz";
    fpsValues << 0.0 << 15.0 << 30.0 << 60.0 << 90.0;
    rotationStrings << "0°" << "180°";
    rotationValues << 0 << 180;

    _resolutionMetaData.setEnumInfo(_resolutionList, resolutionValues);
    _resolutionFact.setMetaData(&_resolutionMetaData);

    _bitRateMetaData.setEnumInfo(bitRateStrings, bitRateValues);
    _bitRateFact.setMetaData(&_bitRateMetaData);

    _fpsMetaData.setEnumInfo(fpsStrings, fpsValues);
    _fpsFact.setMetaData(&_fpsMetaData);

    _rotationMetaData.setEnumInfo(rotationStrings, rotationValues);
    _rotationFact.setMetaData(&_rotationMetaData);

    connect(_vehicle, &Vehicle::mavlinkMessageReceived, this, &VideoStreamManager::_mavlinkMessageReceived);

    _vehicle->sendMavCommand(MAV_COMP_ID_CAMERA,
                             MAV_CMD_REQUEST_VIDEO_STREAM_INFORMATION,
                             false,
                             2.0f,
                             1.0f);
}

VideoStreamManager::~VideoStreamManager()
{

}

void VideoStreamManager::saveSettings(QString ip, int port)
{
    mavlink_message_t msg;

    // Get horizontal and vertical resolution
    QStringList resolutions = resolutionFact()->enumStringValue().split("x");

    if (resolutions.count() == 2) {
        resolutionHorizontalFact()->setRawValue(resolutions[0].toInt());
        resolutionVerticalFact()->setRawValue(resolutions[1].toInt());
    } else if (resolutions[0].contains("auto")) {
        resolutionHorizontalFact()->setRawValue(0);
        resolutionVerticalFact()->setRawValue(0);
    }

    QString uri = QString("udp:%1:%2").arg(ip).arg(QString::number(port));

    mavlink_msg_set_video_stream_settings_pack(_mavlink->getSystemId(),
                                               _mavlink->getComponentId(),
                                               &msg, _vehicle->id(), MAV_COMP_ID_CAMERA, CAMERA_ID,
                                               fpsFact()->rawValue().toFloat(),
                                               resolutionHorizontalFact()->rawValue().toInt(),
                                               resolutionVerticalFact()->rawValue().toInt(),
                                               bitRateFact()->rawValue().toInt(),
                                               rotationFact()->rawValue().toInt(),
                                               uri.toLatin1());

    _vehicle->sendMessageOnLink(_vehicle->priorityLink(), msg);
}

void VideoStreamManager::startVideo()
{
    _vehicle->sendMavCommand(MAV_COMP_ID_CAMERA, MAV_CMD_VIDEO_START_STREAMING, false, CAMERA_ID);
}

void VideoStreamManager::stopVideo()
{
    _vehicle->sendMavCommand(MAV_COMP_ID_CAMERA, MAV_CMD_VIDEO_STOP_STREAMING, false, CAMERA_ID);
}

QString VideoStreamManager::controllerSource()
{
    return QStringLiteral("/qml/VideoStreamControl.qml");
}

QString VideoStreamManager::getLocalAddress()
{
    QString localAddress("127.0.0.1");

    foreach (const QHostAddress &address, QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost)) {
             if (address.isInSubnet(QHostAddress::parseSubnet("192.168.0.0/24"))) {
                localAddress = address.toString();
             }
        }
    }

    return localAddress;
}

void VideoStreamManager::_mavlinkMessageReceived(const mavlink_message_t &message)
{
    switch (message.msgid) {
        case MAVLINK_MSG_ID_VIDEO_STREAM_INFORMATION:
             _handleVideoStreamInfo(message);
    }
}

void VideoStreamManager::_handleVideoStreamInfo(const mavlink_message_t &message)
{
    mavlink_video_stream_information_t videoStreamInformation;
    mavlink_msg_video_stream_information_decode(&message, &videoStreamInformation);

    _videoStatusFact.setRawValue(videoStreamInformation.status);
    _resolutionHorizontalFact.setRawValue(videoStreamInformation.resolution_h);
    _resolutionVerticalFact.setRawValue(videoStreamInformation.resolution_v);
    _bitRateFact.setRawValue(videoStreamInformation.bitrate);
    _fpsFact.setRawValue(videoStreamInformation.framerate);
    _rotationFact.setRawValue(videoStreamInformation.rotation);

    QString resolution = QString("%1x%2")
            .arg(QString::number(videoStreamInformation.resolution_h))
            .arg(QString::number(videoStreamInformation.resolution_v));
    int index = _resolutionList.indexOf(resolution);

    if (index >= 0) {
        _resolutionFact.setRawValue(index);
    } else {
        qDebug() << "Get undefined resolution";
        _resolutionFact.setRawValue(0);
    }
}
