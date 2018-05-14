#include "WifiManagerBase.h"
#include "QGCMAVLink.h"


int WifiNetworkInfo::ssidMaxLength(void)
{
    return sizeof(mavlink_wifi_network_add_t::ssid);
}


int WifiNetworkInfo::passwordMaxLength(void)
{
    return sizeof(mavlink_wifi_network_add_t::password);
}


int WifiNetworkInfo::passwordMinLength(void)
{
    return 8;
}


WifiManagerBase::WifiManagerBase(QObject* parent)
    : QObject(parent),
      _wifiState(WifiState::Undefined),
      _activeNetworkSsid("Undefined")
{ }


bool WifiManagerBase::switchToAccessPoint(void)
{
    switch (_wifiState) {
        case WifiState::AccessPoint:
            _setErrorString("Wifi already in AP");
            return false;

        case WifiState::Switching:
            _setErrorString("Wifi in Switching state");
            return false;

        default:
            if (_switchToAccessPoint()) {
                _setWifiState(WifiState::Switching);
                return true;
            } else {
                return false;
            }
    }
}


bool WifiManagerBase::switchToClient(QString const& ssid)
{
    switch (_wifiState) {
        case WifiState::Client:
            _setErrorString("Wifi already in Client mode");
            return false;

        case WifiState::Switching:
            _setErrorString("Wifi in Switching state");
            return false;

        default:
            break;
    }

    if (!_listContainsNetwork(ssid)) {
        _setErrorString("Can not connect to unsaved network");
        return false;
    }

    if (_switchToClient(ssid)) {
        _setWifiState(WifiState::Switching);
        return true;
    }

    return false;
}


bool WifiManagerBase::addNetwork(QString const& ssid,
                                 QString const& passwd,
                                 WifiNetworkInfo::SecurityType secType)
{
    if (_listContainsNetwork(ssid)) {
        auto msg = QString("Can not add %1 network. "
                           "Network with same ssid already exists").arg(ssid);
        _setErrorString(msg);
        return false;
    }

    if (ssid.length() > WifiNetworkInfo::ssidMaxLength() || ssid.isEmpty()) {
        auto msg = QString("Ssid is bigger than maximum %1 or empty")
                       .arg(WifiNetworkInfo::ssidMaxLength());
        _setErrorString(msg);
        return false;
    }

    if (passwd.length() > WifiNetworkInfo::passwordMaxLength()
     || passwd.length() < WifiNetworkInfo::passwordMinLength())
    {
        auto msg = QString("Password is bigger than %1 or less than %2")
                      .arg(WifiNetworkInfo::passwordMaxLength())
                      .arg(WifiNetworkInfo::passwordMinLength());
        _setErrorString(msg);
        return false;
    }

    return _addNetwork(ssid, passwd, secType);
}


bool WifiManagerBase::deleteNetwork(QString const& ssid)
{
    if (!_listContainsNetwork(ssid)) {
        auto msg = QString("Can not delete network %1. "
                           "Network is not saved.").arg(ssid);
        _setErrorString(msg);
        return false;
    }

    if (ssid == _activeNetworkSsid) {
        auto msg = QString("Can not delete network %1. "
                           "Network is active.").arg(ssid);
        _setErrorString(msg);
    }

    return _deleteNetwork(ssid);
}
