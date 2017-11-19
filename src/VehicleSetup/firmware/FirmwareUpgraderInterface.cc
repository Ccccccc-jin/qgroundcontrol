#include "FirmwareUpgraderInterface.h"
#include "MockFirmwareUpgrader.h"
#include "EdgeFirmwareUpgrader.h"



FirmwareUpgrader::FirmwareUpgrader(QObject* parent)
    : QObject(parent)
{  }


FirmwareUpgrader::~FirmwareUpgrader(void)
{  }


std::unique_ptr<FirmwareUpgrader> FirmwareUpgrader::instance() {
    return std::unique_ptr<FirmwareUpgrader>(new EdgeFirmwareUpgrader());
}
