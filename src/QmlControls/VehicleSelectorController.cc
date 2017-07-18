/****************************************************************************
 *
 *   (c) 2009-2017 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "VehicleSelectorController.h"
#include "QGCMAVLink.h"
#include "QGCApplication.h"

VehicleSelectorController::VehicleSelectorController(void)
    : _currentVehicleType(-1)
{
    QStringList names, imgSources;
    QList<int> vehicleIDs;

    names << "arduplane" << "arducopter" << "arducopter-heli" << "ardurover" << "ardusub";
    imgSources << "qrc:/qmlimages/Airframe/Plane";
    imgSources << "qrc:/qmlimages/Airframe/QuadRotorX";
    imgSources << "qrc:/qmlimages/Airframe/Helicopter";
    imgSources << "qrc:/qmlimages/Airframe/Rover";
    imgSources << "qrc:/qmlimages/Airframe/AirframeUnknown";
    vehicleIDs << 0 << 10 << 11 << 20 << 30;

    for (int i=0; i<names.length(); i++) {
        VehicleSelector* vehicleType = new VehicleSelector(names[i], imgSources[i], vehicleIDs[i], this);
        Q_CHECK_PTR(vehicleType);
        _vehicleTypes.append(QVariant::fromValue(vehicleType));
    }
}

VehicleSelectorController::~VehicleSelectorController()
{

}

void VehicleSelectorController::changeVehicle(void)
{
    if (qgcApp()->toolbox()->multiVehicleManager()->vehicles()->count() > 1) {
        qgcApp()->showMessage("You cannot change airframe configuration while connected to multiple vehicles.");
        return;
    }

    qgcApp()->setOverrideCursor(Qt::WaitCursor);

    Fact* vehicleFact  = getParameterFact(-1, "SYSID_SW_TYPE");

    // We need to wait for the vehicleUpdated signals to come back before we reboot
    connect(vehicleFact, &Fact::vehicleUpdated, this, &VehicleSelectorController::_rebootAfterStackUnwind);

    if (_currentVehicleType < 0) return;

    vehicleFact->forceSetRawValue(_currentVehicleType);
}

void VehicleSelectorController::_rebootAfterStackUnwind(void)
{
    _vehicle->sendMavCommand(_vehicle->defaultComponentId(), MAV_CMD_PREFLIGHT_REBOOT_SHUTDOWN, true /* showError */, 1.0f);
    qgcApp()->processEvents(QEventLoop::ExcludeUserInputEvents);
    for (unsigned i = 0; i < 2000; i++) {
        QGC::SLEEP::usleep(500);
        qgcApp()->processEvents(QEventLoop::ExcludeUserInputEvents);
    }
    qgcApp()->toolbox()->linkManager()->disconnectAll();
    qgcApp()->restoreOverrideCursor();
}

VehicleSelector::VehicleSelector(const QString& name, const QString& imgSource, int vehicleID,  QObject* parent) :
    QObject(parent),
    _name(name),
    _imgSource(imgSource),
    _vehicleID(vehicleID)
{

}

VehicleSelector::~VehicleSelector()
{

}
