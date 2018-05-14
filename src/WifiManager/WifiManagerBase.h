#ifndef WIFIMANAGERBASE_H
#define WIFIMANAGERBASE_H

#include <QtCore>


class WifiNetworkInfo
{
public:
    enum SecurityType {
        Open,
        WEP,
        WPA,
        WPA2
    };
    Q_ENUMS(SecurityType)

    WifiNetworkInfo(QString ssid,
                    SecurityType secType = SecurityType::Open)
        : _ssid(std::move(ssid)),
          _secType(secType)
    { }

    bool operator ==(WifiNetworkInfo const& info) { return _ssid == info._ssid; }
    bool operator !=(WifiNetworkInfo const& info) { return !(*(this) == info); }

    QString const&                ssid(void)         const { return _ssid; }
    WifiNetworkInfo::SecurityType securityType(void) const { return _secType; }

    static int ssidMaxLength(void);
    static int passwordMaxLength(void);
    static int passwordMinLength(void);

private:
    QString      _ssid;
    SecurityType _secType;
};


class WifiManagerBase : public QObject
{
    Q_OBJECT
public:
    enum WifiState : int8_t
    {
        Undefined = 0,
        AccessPoint,
        Client,
        Switching
    };
    Q_ENUMS(WifiState)

    virtual ~WifiManagerBase(void) = default;

    WifiState const&                  wifiState(void)             const { return _wifiState; }
    QString const&                    activeNetworkSsid(void)     const { return _activeNetworkSsid; }
    std::list<WifiNetworkInfo> const& savedNetworksInfoList(void) const { return _savedNetworksInfoList; }

    bool switchToAccessPoint(void);
    bool switchToClient(QString const& ssid);

    bool addNetwork(QString const& ssid,
                    QString const& passwd,
                    WifiNetworkInfo::SecurityType secType);

    bool deleteNetwork(QString const& ssid);

    QString const& errorString(void) const { return _errorString; }

signals:
    void wifiStateChanged(void);
    void activeNetworkSsidChanged(void);
    void savedNetworksListChanged(void);

protected:
    WifiManagerBase(QObject* parent = nullptr);


    void _setWifiState(WifiState state) {
        _wifiState = state;
        emit wifiStateChanged();
    }

    void _setErrorString(QString errorString) {
        qWarning() << errorString;
        _errorString = std::move(errorString);
    }

    void _setActiveNetworkSsid(QString activeNetworkSsid) {
        _activeNetworkSsid = std::move(activeNetworkSsid);
        emit activeNetworkSsidChanged();
    }

    void _addNetworkToList(WifiNetworkInfo netwkInfo) {
        _savedNetworksInfoList.push_back(std::move(netwkInfo));
        emit savedNetworksListChanged();
    }

    void _removeNetworkFromList(QString const& ssid) {
        _savedNetworksInfoList
                .remove_if([&ssid] (WifiNetworkInfo const& netwk) { return netwk.ssid() == ssid; });
        emit savedNetworksListChanged();
    }

    void _clearSavedNetworksInfoList(void) {
        _savedNetworksInfoList.clear();
        emit savedNetworksListChanged();
    }

    bool _listContainsNetwork(QString const& ssid)
    {
        auto const& list = _savedNetworksInfoList;

        auto condition = [&ssid] (WifiNetworkInfo const& info) { return info.ssid() == ssid; };
        auto netwk = std::find_if(list.cbegin(), list.cend(), condition);

        return netwk != list.cend();
    }

private:
    // Keep implemetation of logic for children

    virtual bool _switchToAccessPoint(void) = 0;
    virtual bool _switchToClient(QString const& ssid) = 0;

    virtual bool _addNetwork(QString const& ssid,
                             QString const& passwd,
                             WifiNetworkInfo::SecurityType secType) = 0;

    virtual bool _deleteNetwork(QString const& ssid) = 0;

    WifiState                  _wifiState;
    QString                    _activeNetworkSsid;
    std::list<WifiNetworkInfo> _savedNetworksInfoList;

    QString                    _errorString;
};


#endif
