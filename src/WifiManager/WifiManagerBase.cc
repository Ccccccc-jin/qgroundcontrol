#include "WifiManagerBase.h"
#include "QGCMAVLink.h"


WifiManagerBase::WifiManagerBase(QObject* parent)
    : QObject(parent),
      _wifiState(WifiState::Undefined),
      _activeNetworkSsid("Undefined")
{
    QSettings appSettings;
    if (appSettings.contains("WifiSetup/EdgeDefaultNetwork")) {
        _defaultNetworkSsid = appSettings
                .value("WifiSetup/EdgeDefaultNetwork")
                .toString();
    }
}


void WifiManagerBase::setDefaultNetworkSsid(QString ssid)
{
    QSettings appSettings;

    appSettings.setValue("WifiSetup/EdgeDefaultNetwork", ssid);
    _defaultNetworkSsid = std::move(ssid);

    appSettings.sync();

    emit defaultNetworkSsidChanged();
}


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


bool WifiManagerBase::validatePassword(QString const& passwd)
{
    /// We have ASCII PSK (max 63 bytes length) and HEX (fixed 64 bytes length)
    auto checkOnAsciiPSK = passwd.length() <  WifiNetworkInfo::passwordMaxLength();
    auto checkOnHexPSK   = passwd.length() == WifiNetworkInfo::passwordMinLength();

    if (checkOnAsciiPSK) {
        return passwd.length() >= WifiNetworkInfo::passwordMinLength();

    } else if (checkOnHexPSK) {
        auto hexadecimalPattern = QRegExp("^[0-9A-Fa-f]*$");
        return passwd.contains(hexadecimalPattern);
    }

    return false;
}


QString WifiManagerBase::wifiStateAsString(WifiState state) const
{
    static auto wifiStateMeta = QMetaEnum::fromType<WifiManagerBase::WifiState>();
    return wifiStateMeta.valueToKey(state);
}


QString WifiManagerBase::securityTypeAsString(WifiNetworkInfo::SecurityType secType) const
{
    static auto secTypeMeta = QMetaEnum::fromType<WifiNetworkInfo::SecurityType>();
    return secTypeMeta.valueToKey(secType);
}
