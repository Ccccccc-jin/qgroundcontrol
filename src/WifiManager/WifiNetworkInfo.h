#ifndef WIFINETWORKINFO_H
#define WIFINETWORKINFO_H

#include <QtCore>

class WifiNetworkInfo
{
    Q_GADGET
public:
    enum SecurityType : int8_t {
        Open = 0,
        WEP,
        WPA,
        WPA2
    };
    Q_ENUM(SecurityType)

    WifiNetworkInfo(QString ssid,
                    SecurityType secType = SecurityType::Open)
        : _ssid(std::move(ssid)),
          _secType(secType)
    { }

    bool operator ==(WifiNetworkInfo const& info) { return _ssid == info._ssid; }
    bool operator !=(WifiNetworkInfo const& info) { return !(*(this) == info); }

    QString const&                ssid(void)         const { return _ssid; }
    WifiNetworkInfo::SecurityType securityType(void) const { return _secType; }

    Q_INVOKABLE static int ssidMaxLength(void);
    Q_INVOKABLE static int passwordMaxLength(void);
    Q_INVOKABLE static int passwordMinLength(void);

private:
    QString      _ssid;
    SecurityType _secType;
};

#endif
