#include "WifiNetworksList.h"

using Base = QAbstractListModel;

WifiNetworksList::WifiNetworksList(QObject* parent)
    : QAbstractListModel(parent)
{ }


QHash<int, QByteArray> WifiNetworksList::roleNames(void) const
{
    return {
        {NetworkRole::SsidRole,           "ssid"},
        {NetworkRole::EncryptionTypeRole, "encryptionType"}
    };
}


void WifiNetworksList::add(WifiNetworkInfo network)
{
    Base::beginInsertRows(QModelIndex(), rowCount(), rowCount());
    _networks.push_back(std::move(network));
    Base::endInsertRows();
}


void WifiNetworksList::remove(QString const& ssid)
{
}


bool WifiNetworksList::contains(QString const& ssid)
{
    auto const& list = _networks;

    auto condition = [&ssid] (WifiNetworkInfo const& info) { return info.ssid() == ssid; };
    auto netwk = std::find_if(list.cbegin(), list.cend(), condition);

    return netwk != list.cend();
}


void WifiNetworksList::clear(void)
{
    Base::beginRemoveRows(QModelIndex(), 0, rowCount());
    _networks.clear();
    Base::endRemoveRows();
}


int WifiNetworksList::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return _networks.size();
}


QVariant WifiNetworksList::data(const QModelIndex &index, int role) const
{
    auto currentRow = index.row();

    if (currentRow < 0 || currentRow > _networks.size()) {
        return {};
    }

    auto const& network = _networks[currentRow];

    if (role == NetworkRole::SsidRole) {
        return network.ssid();
    } else if (role == NetworkRole::EncryptionTypeRole) {
        return network.securityType();
    }

    return {};
}
