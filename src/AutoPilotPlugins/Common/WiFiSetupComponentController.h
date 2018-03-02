/****************************************************************************
 *
 *   (c) 2009-2017 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/


#ifndef WiFiSetupComponentController_H
#define WiFiSetupComponentController_H

#include <QObject>
#include <QList>

#include "FactPanelController.h"

class WifiNetwork {
public:
    WifiNetwork(QString const& ssid, QString const& encryptType)
        : _ssid(ssid), _encryptType(encryptType)
    { }

    QString ssid(void) const& { return _ssid; }
    QString encryptionType(void) const& { return _encryptType; }

private:
    QString _ssid, _encryptType;
};

class WifiNetworksListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum NetworkRole {
        SsidRole = Qt::UserRole + 1,
        EncryptionTypeRole
    };

    WifiNetworksListModel(QObject* parent = nullptr);

    void add(WifiNetwork const& network);
    void clear(void);

    QHash<int, QByteArray> roleNames(void) const override;

    int rowCount(QModelIndex const& parent = QModelIndex()) const override;

    QVariant data(QModelIndex const& index, int role = Qt::DisplayRole) const override;

private:
    QList<WifiNetwork> _networks;
};


class WiFiSetupComponentController : public FactPanelController
{
    Q_OBJECT

public:
    enum EdgeMode : int { AccessPoint = 0, Client, Undefined };
    Q_ENUMS(EdgeMode)

    enum EncryptionType : int { OpenEncrypt = 0, WepEncrypt, WpaEncrypt, Wpa2Encrypt };
    Q_ENUMS(EncryptionType)

    WiFiSetupComponentController(void);
    ~WiFiSetupComponentController(void) = default;

    Q_PROPERTY(QStringList savedNetworks   MEMBER _savedNetworks  NOTIFY savedNetworksUpdated)
    Q_PROPERTY(EdgeMode    edgeMode        MEMBER _edgeMode       NOTIFY edgeModeChanged)
    Q_PROPERTY(QString     defaultNetwork  MEMBER _defaultNetwork WRITE  setDefaultNetwork NOTIFY defaultNetworkChanged)
    Q_PROPERTY(QString     activeNetwork   MEMBER _activeNetwork  NOTIFY activeNetworkChanged)

    Q_PROPERTY(QStringList encryptTypeStrings  MEMBER _encryptTypeStrings   CONSTANT)
    Q_PROPERTY(int         ssidMaxLength       MEMBER _ssidMaxLength        CONSTANT)
    Q_PROPERTY(int         passwdMaxLength     MEMBER _passwdMaxLength      CONSTANT)

    Q_PROPERTY(WifiNetworksListModel* scannedNetworks READ scannedNetworks NOTIFY scannedNetworksUpdated)

    Q_INVOKABLE void bootAsAccessPoint     (void);
    Q_INVOKABLE void bootAsClient          (QString const& netwkName);

    Q_INVOKABLE void removeNetworkFromEdge (QString const& netwkName);
    Q_INVOKABLE void saveNetworkToEdge     (QString const& netwkName, int netwkType, QString const& netwkPasswd);

    Q_INVOKABLE QString getSavedNetwork(int idx) { return _savedNetworks[idx]; }

    Q_INVOKABLE QString encryptionTypeAsString(EncryptionType type) {
        return _encryptTypeStrings[static_cast<int>(type)];
    }

    WifiNetworksListModel* scannedNetworks(void) { return &_scannedNetworks; }
    void setDefaultNetwork(QString const& network);

signals:
    void savedNetworksUpdated  (void);
    void edgeModeChanged       (void);
    void defaultNetworkChanged (void);
    void activeNetworkChanged  (void);
    void scannedNetworksUpdated(void);

private slots:
    void _handleWiFiNetworkInformation(mavlink_message_t message);
    void _handleWifiStatus(mavlink_message_t message);

private:
    void _requestWifiStatus(void);
    void _updateSavedNetworksList(void);

    QStringList _encryptTypeStrings;
    QStringList _savedNetworks;

    QString  _defaultNetwork;
    QString  _activeNetwork;

    EdgeMode _edgeMode;
    WifiNetworksListModel _scannedNetworks;

    static int const _ssidMaxLength;
    static int const _passwdMaxLength;
};

#endif // WiFiSetupComponentController_H
