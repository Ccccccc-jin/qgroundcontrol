/****************************************************************************
 *
 *   (c) 2009-2017 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/


#include "WiFiSetupComponent.h"
#include "AutoPilotPlugin.h"

WiFiSetupComponent::WiFiSetupComponent(Vehicle* vehicle, AutoPilotPlugin* autopilot, QObject* parent)
    : VehicleComponent(vehicle, autopilot, parent)
    , _name(tr("Wi-Fi"))
{

}

QString WiFiSetupComponent::name(void) const
{
    return _name;
}

QString WiFiSetupComponent::description(void) const
{
    return tr("The Wi-Fi Setup Component is used to setup the WiFi link.");
}

QString WiFiSetupComponent::iconResource(void) const
{
    return "/qmlimages/wifi.svg";
}

bool WiFiSetupComponent::requiresSetup(void) const
{
    return false;
}

bool WiFiSetupComponent::setupComplete(void) const
{
    return true;
}

QStringList WiFiSetupComponent::setupCompleteChangedTriggerList(void) const
{
    return QStringList();
}

QUrl WiFiSetupComponent::setupSource(void) const
{
    return QUrl::fromUserInput("qrc:/qml/WiFiSetupComponent.qml");
}

QUrl WiFiSetupComponent::summaryQmlSource(void) const
{
    return QUrl::fromUserInput("qrc:/qml/WiFiSetupComponentSummary.qml");
}
