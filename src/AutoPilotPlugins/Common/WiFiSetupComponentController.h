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

class WiFiSetupComponentController : public FactPanelController
{
    Q_OBJECT

public:
    WiFiSetupComponentController(void);
    ~WiFiSetupComponentController(void);

    Q_PROPERTY(QStringList networks MEMBER _networks NOTIFY _networksChanged)
    Q_PROPERTY(QStringList protocolTypes MEMBER _protocolTypes CONSTANT)
    Q_PROPERTY(QString connectionName MEMBER _connectionName CONSTANT)
    Q_PROPERTY(QString connectionType MEMBER _connectionType CONSTANT)

    Q_INVOKABLE void startAPMode();

    Q_INVOKABLE void connectToNetwork(const QString name);

    Q_INVOKABLE void addNetwork(const QString name, const int type, const QString psw);

    Q_INVOKABLE void removeNetwork(const QString name);

signals:
    void _networksChanged();

private slots:
    void _handleWiFiNetworkInformation(mavlink_message_t message);

private:
    void update_network_list();

    QStringList _protocolTypes;
    QStringList _networks;

    QString _connectionName;
    QString _connectionType;
};

#endif // WiFiSetupComponentController_H
