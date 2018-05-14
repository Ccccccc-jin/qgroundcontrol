#include "WifiNetworkInfo.h"
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
