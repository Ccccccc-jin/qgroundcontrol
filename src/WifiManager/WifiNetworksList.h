#ifndef WIFINETWORKSLIST_H
#define WIFINETWORKSLIST_H

#include <QtCore>
#include <memory>

#include "WifiNetworkInfo.h"

class WifiNetworksList : public QAbstractListModel
{
    Q_OBJECT
public:
    enum NetworkRole {
        SsidRole = Qt::UserRole + 1,
        EncryptionTypeRole
    };

    WifiNetworksList(QObject* parent = nullptr);

    void add(WifiNetworkInfo network);

    void remove   (QString const& ssid);
    bool contains (QString const& ssid);

    void clear(void);

    QHash<int, QByteArray> roleNames(void) const override;

    int rowCount(QModelIndex const& parent = QModelIndex()) const override;

    QVariant data(QModelIndex const& index, int role = Qt::DisplayRole) const override;

private:
    QList<WifiNetworkInfo> _networks;
};

#endif
