/****************************************************************************
 *
 *   (c) 2009-2017 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include "QGCApplication.h"
#include <QLoggingCategory>
#include "QmlObjectListModel.h"
#include "Vehicle.h"
#include "FactPanelController.h"

#include <QObject>
#include <QTimer>

class Vehicle;

class VideoStreamManager : public QObject
{
    Q_OBJECT
public:
    VideoStreamManager(Vehicle* vehicle);

    ~VideoStreamManager();

    Q_PROPERTY(QString controllerSource       READ controllerSource         NOTIFY controllerSourceChanged)
    Q_PROPERTY(QString targetIp               READ targetIp                 NOTIFY addressChanged)
    Q_PROPERTY(int targetPort                 READ targetPort               NOTIFY addressChanged)
    Q_PROPERTY(Fact* videoStatusFact          READ videoStatusFact          CONSTANT)
    Q_PROPERTY(Fact* resolutionFact           READ resolutionFact           CONSTANT)
    Q_PROPERTY(Fact* resolutionHorizontalFact READ resolutionHorizontalFact CONSTANT)
    Q_PROPERTY(Fact* resolutionVerticalFact   READ resolutionVerticalFact   CONSTANT)
    Q_PROPERTY(Fact* bitRateFact              READ bitRateFact              CONSTANT)
    Q_PROPERTY(Fact* fpsFact                  READ fpsFact                  CONSTANT)
    Q_PROPERTY(Fact* rotationFact             READ rotationFact             CONSTANT)

    Q_INVOKABLE void saveSettings(QString ip, int port);
    Q_INVOKABLE void startVideo();
    Q_INVOKABLE void stopVideo();
    Q_INVOKABLE QString getLocalAddress();

    //-- Camera controller source (QML)
    virtual QString  controllerSource    ();

    QString targetIp(void) { return _targetIp; }
    int   targetPort(void) { return _targetPort; }

    Fact* videoStatusFact(void) { return &_videoStatusFact; }
    Fact* resolutionFact(void)  { return &_resolutionFact; }
    Fact* resolutionHorizontalFact(void) { return &_resolutionHorizontalFact; }
    Fact* resolutionVerticalFact(void) { return &_resolutionVerticalFact; }
    Fact* bitRateFact(void)     { return &_bitRateFact; }
    Fact* fpsFact(void)         { return &_fpsFact; }
    Fact* rotationFact(void)    { return &_rotationFact; }

private slots:
    void    _mavlinkMessageReceived (const mavlink_message_t& message);

signals:
    void    controllerSourceChanged();
    void    addressChanged();

private:
    void    _handleVideoStreamInfo (const mavlink_message_t& message);

private:
    Vehicle*         _vehicle;
    MAVLinkProtocol* _mavlink;

    QStringList _resolutionList;

    QString _targetIp;
    int     _targetPort;
    Fact    _videoStatusFact;
    Fact    _resolutionFact;
    Fact    _resolutionHorizontalFact;
    Fact    _resolutionVerticalFact;
    Fact    _bitRateFact;
    Fact    _fpsFact;
    Fact    _rotationFact;

    FactMetaData _resolutionMetaData;
    FactMetaData _bitRateMetaData;
    FactMetaData _fpsMetaData;
    FactMetaData _rotationMetaData;
};
