#include "VehicleBatteries.h"


const char* VehicleBatteryFactGroup::_voltageFactName =                     "voltage";
const char* VehicleBatteryFactGroup::_percentRemainingFactName =            "percentRemaining";
const char* VehicleBatteryFactGroup::_mahConsumedFactName =                 "mahConsumed";
const char* VehicleBatteryFactGroup::_currentFactName =                     "current";
const char* VehicleBatteryFactGroup::_temperatureFactName =                 "temperature";
const char* VehicleBatteryFactGroup::_cellCountFactName =                   "cellCount";

const char* VehicleBatteryFactGroup::_settingsGroup =                       "Vehicle.battery";

const double VehicleBatteryFactGroup::_voltageUnavailable =           -1.0;
const int    VehicleBatteryFactGroup::_percentRemainingUnavailable =  -1;
const int    VehicleBatteryFactGroup::_mahConsumedUnavailable =       -1;
const int    VehicleBatteryFactGroup::_currentUnavailable =           -1;
const double VehicleBatteryFactGroup::_temperatureUnavailable =       -1.0;
const int    VehicleBatteryFactGroup::_cellCountUnavailable =         -1;

VehicleBatteryFactGroup::VehicleBatteryFactGroup(QObject* parent)
    : FactGroup(1000, ":/json/Vehicle/BatteryFact.json", parent)
    , _voltageFact                  (0, _voltageFactName,                   FactMetaData::valueTypeDouble)
    , _percentRemainingFact         (0, _percentRemainingFactName,          FactMetaData::valueTypeInt32)
    , _mahConsumedFact              (0, _mahConsumedFactName,               FactMetaData::valueTypeInt32)
    , _currentFact                  (0, _currentFactName,                   FactMetaData::valueTypeFloat)
    , _temperatureFact              (0, _temperatureFactName,               FactMetaData::valueTypeDouble)
    , _cellCountFact                (0, _cellCountFactName,                 FactMetaData::valueTypeInt32)
{
    _addFact(&_voltageFact,                 _voltageFactName);
    _addFact(&_percentRemainingFact,        _percentRemainingFactName);
    _addFact(&_mahConsumedFact,             _mahConsumedFactName);
    _addFact(&_currentFact,                 _currentFactName);
    _addFact(&_temperatureFact,             _temperatureFactName);
    _addFact(&_cellCountFact,               _cellCountFactName);

    // Start out as not available
    _voltageFact.setRawValue            (_voltageUnavailable);
    _percentRemainingFact.setRawValue   (_percentRemainingUnavailable);
    _mahConsumedFact.setRawValue        (_mahConsumedUnavailable);
    _currentFact.setRawValue            (_currentUnavailable);
    _temperatureFact.setRawValue        (_temperatureUnavailable);
    _cellCountFact.setRawValue          (_cellCountUnavailable);
}

void VehicleBatteryFactGroup::setMahConsumed(int32_t value)
{
    mahConsumed()->setRawValue(value);
}

void VehicleBatteryFactGroup::setCellCount(int32_t value)
{
    cellCount()->setRawValue(value);
}

void VehicleBatteryFactGroup::setCurrent(float value)
{
    current()->setRawValue(value == -1.0 ? _currentUnavailable : value / 100.0f);
}

void VehicleBatteryFactGroup::setVoltage(double value)
{
   voltage()->setRawValue(value >= static_cast<double>(UINT16_MAX) ?
       VehicleBatteryFactGroup::_voltageUnavailable : value / 1000.0);
}

void VehicleBatteryFactGroup::setTemperature(double value)
{
    temperature()->setRawValue(value == static_cast<double>(INT16_MAX) ?
        VehicleBatteryFactGroup::_temperatureUnavailable : value / 100.0);
}

void VehicleBatteryFactGroup::setPercentRemaining(int32_t value) {
    percentRemaining()->setRawValue(value);
}

VehicleBatteriesFactGroup::VehicleBatteriesFactGroup(QObject *parent)
    : FactGroup{1000,"",parent}
{
    _addFactGroup(new VehicleBatteryFactGroup(this), "Battery 1");
    _addFactGroup(new VehicleBatteryFactGroup(this), "Battery 2");
}

void VehicleBatteriesFactGroup::handleBatteryStatus(const mavlink_message_t& message)
{
    mavlink_battery_status_t battStatus;
    mavlink_msg_battery_status_decode(&message, &battStatus);

    auto batteryName = _getBatteryName(battStatus.id);
    _appendBatteryIfNotPresent(batteryName);
    auto battery = dynamic_cast<VehicleBatteryFactGroup*>(getFactGroup(batteryName));

    battery->setMahConsumed(battStatus.current_consumed);
    battery->setTemperature(battStatus.temperature);
    battery->setPercentRemaining (battStatus.battery_remaining);
}

void VehicleBatteriesFactGroup::handleSysStatus(const mavlink_message_t& message)
{
    const int mainBatteryId = 0;
    mavlink_sys_status_t sysStatus;
    mavlink_msg_sys_status_decode(&message, &sysStatus);

    auto batteryName = _getBatteryName(mainBatteryId);
    _appendBatteryIfNotPresent(batteryName);
    auto battery = dynamic_cast<VehicleBatteryFactGroup*>(getFactGroup(batteryName));

    battery->setCurrent(sysStatus.current_battery);
    battery->setVoltage(sysStatus.voltage_battery);
    battery->setPercentRemaining (sysStatus.battery_remaining);
}

void VehicleBatteriesFactGroup::handleBattery2(const mavlink_message_t& message)
{
    const int secondBatteryId = 1;
    mavlink_battery2_t batt2;
    mavlink_msg_battery2_decode(&message, &batt2);

    auto batteryName = _getBatteryName(secondBatteryId);
    _appendBatteryIfNotPresent(batteryName);
    auto battery = dynamic_cast<VehicleBatteryFactGroup*>(getFactGroup(batteryName));

    battery->setCurrent(batt2.current_battery);
    battery->setVoltage(batt2.voltage);
}

QString VehicleBatteriesFactGroup::_getBatteryName(int batteryId)
{
   return QStringLiteral("Battery %1").arg(batteryId + 1);
}

void VehicleBatteriesFactGroup::_appendBatteryIfNotPresent(QString const& batteryName)
{
    if (!getFactGroup(batteryName)) {
        _addFactGroup(new VehicleBatteryFactGroup(this), batteryName);
    }
}

VehicleBatteriesFactGroup::~VehicleBatteriesFactGroup(void) {}
