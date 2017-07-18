/****************************************************************************
 *
 *   (c) 2009-2017 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/


#ifndef VehicleSelectorController_H
#define VehicleSelectorController_H

#include <QObject>
#include <QList>

#include "FactPanelController.h"

class VehicleSelectorController : public FactPanelController
{
    Q_OBJECT

public:
    Q_PROPERTY(QVariantList vehicleTypes MEMBER _vehicleTypes CONSTANT)
    Q_PROPERTY(int currentVehicleType MEMBER _currentVehicleType NOTIFY vehicleTypeChanged)

    Q_INVOKABLE void changeVehicle();

    VehicleSelectorController(void);
    ~VehicleSelectorController();

signals:
    void vehicleTypeChanged(int newVehicleType);

private slots:
    void _rebootAfterStackUnwind(void);

private:
    QVariantList _vehicleTypes;
    int          _currentVehicleType;
};

class VehicleSelector : public QObject
{
    Q_OBJECT

public:
    VehicleSelector(const QString& name, const QString& imgSource, int vehicleID,  QObject* parent);
    ~VehicleSelector();

    Q_PROPERTY(QString name MEMBER _name CONSTANT)
    Q_PROPERTY(QString imgSource MEMBER _imgSource CONSTANT)
    Q_PROPERTY(int vehicleID MEMBER _vehicleID CONSTANT)

private:
    QString _name;
    QString _imgSource;
    int     _vehicleID;
};

#endif
