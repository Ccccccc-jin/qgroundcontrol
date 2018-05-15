#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <QtCore>
#include <queue>

#include "WifiManagerBase.h"
#include "QGCMAVLink.h"


class Vehicle;

// For QVariantList
Q_DECLARE_METATYPE(WifiNetworkInfo::SecurityType)

class WifiManager : public WifiManagerBase
{
    Q_OBJECT
public:
    WifiManager(Vehicle* vehicle, QObject* parent = nullptr);
    ~WifiManager(void) = default;

    void reloadWifiInfoFromVehicle(void) {
        _requestWifiStatus();
        _requestSavedNetworksCount();
    }

signals:
    void _savedNetworksCountReceived(void);

private:
    enum NetworkType : int8_t {
        Saved = 0,
        Scanned
    };

    bool _switchToAccessPoint(void) override;
    bool _switchToClient(QString const& ssid) override;

    bool _addNetwork(QString const& ssid,
                     QString const& passwd,
                     WifiNetworkInfo::SecurityType secType) override;

    bool _deleteNetwork(QString const& ssid) override;


    void _onMavlinkMessageReceived(mavlink_message_t const& msg);
    void _onNetworksListRequestTimeout(void);
    void _onAckTimerTimeout           (void);
    void _onSavedNetworksCountReceived(void);

    void _requestWifiStatus         (void);
    void _requestSavedNetworksCount (void);
    void _requestSavedNetworksInfo  (void);

    void _handleWifiAck           (mavlink_message_t const& msg);
    void _handleWifiStatus        (mavlink_message_t const& msg);
    void _handleWifiNetworkInfo   (mavlink_message_t const& msg);
    void _handleWifiNetworksCount (mavlink_message_t const& msg);

    void _handleConnectionLost(bool isConnectionLost);
    void _sendMessage(mavlink_message_t msg);

    int                 _savedNetworksCount;
    int                 _receivedSavedNetworksCount;

    QTimer              _ackTimer;
    QTimer              _netwksListRequestTimer;
    int                 _retryCount;
    Vehicle*            _vehicle;

    std::queue<mavlink_message_t> _messageQueue;
};

#endif
