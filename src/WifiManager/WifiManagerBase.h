#ifndef WIFIMANAGERBASE_H
#define WIFIMANAGERBASE_H

#include <QtCore>
#include "WifiNetworksList.h"


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
    Q_ENUM(WifiState)

    Q_PROPERTY(WifiState         wifiState          READ wifiState             NOTIFY wifiStateChanged)
    Q_PROPERTY(QString           activeNetworkSsid  READ activeNetworkSsid     NOTIFY activeNetworkSsidChanged)
    Q_PROPERTY(WifiNetworksList* savedNetworks      READ savedNetworksInfoList CONSTANT)
    Q_PROPERTY(QString           defaultNetworkSsid READ defaultNetworkSsid    WRITE  setDefaultNetworkSsid
                                                                               NOTIFY defaultNetworkSsidChanged)

    virtual ~WifiManagerBase(void) = default;

    WifiState const&  wifiState(void)             const { return _wifiState; }
    QString const&    activeNetworkSsid(void)     const { return _activeNetworkSsid; }
    WifiNetworksList* savedNetworksInfoList(void)       { return &_savedNetworksInfoList; }
    QString const&    defaultNetworkSsid(void)    const { return _defaultNetworkSsid; }

    void setDefaultNetworkSsid(QString ssid);

    Q_INVOKABLE int ssidMaxLength(void);
    Q_INVOKABLE int passwordMaxLength(void);
    Q_INVOKABLE int passwordMinLength(void);

    Q_INVOKABLE bool switchToAccessPoint(void);
    Q_INVOKABLE bool switchToClient(QString const& ssid);

    Q_INVOKABLE bool addNetwork(QString const& ssid,
                                QString const& passwd,
                                WifiNetworkInfo::SecurityType secType);

    Q_INVOKABLE bool deleteNetwork(QString const& ssid);

    Q_INVOKABLE bool validatePassword(QString const& passwd);

    Q_INVOKABLE QString const& errorString(void) const { return _errorString; }

    Q_INVOKABLE QString wifiStateAsString(WifiState state) const;
    Q_INVOKABLE QString securityTypeAsString(WifiNetworkInfo::SecurityType secType) const;

signals:
    void wifiStateChanged(void);
    void activeNetworkSsidChanged(void);
    void savedNetworksListChanged(void);
    void defaultNetworkSsidChanged(void);

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
        _savedNetworksInfoList.add(std::move(netwkInfo));
    }

    void _removeNetworkFromList(QString const& ssid) {
        _savedNetworksInfoList.remove(ssid);
    }

    void _clearSavedNetworksInfoList(void) {
        _savedNetworksInfoList.clear();
    }

    bool _listContainsNetwork(QString const& ssid)
    {
        return _savedNetworksInfoList.contains(ssid);
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
    WifiNetworksList           _savedNetworksInfoList;

    QString                    _errorString;
    QString                    _defaultNetworkSsid;
};


#endif
