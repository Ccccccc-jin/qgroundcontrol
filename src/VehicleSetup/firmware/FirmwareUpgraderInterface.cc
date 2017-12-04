#include "FirmwareUpgraderInterface.h"
#ifdef Q_OS_WIN
#include "upgraders/FirmwareUpgraderClientStub.h"
#else
#include "upgraders/FirmwareUpgraderClient.h"
#endif



FirmwareUpgrader::FirmwareUpgrader(QObject* parent)
    : QObject(parent)
{  }


FirmwareUpgrader::~FirmwareUpgrader(void)
{  }


std::unique_ptr<FirmwareUpgrader> FirmwareUpgrader::instance() {
#if defined(Q_OS_UNIX)
    return std::unique_ptr<FirmwareUpgrader>(new FirmwareUpgraderClient());
#else
    return std::unique_ptr<FirmwareUpgrader>(new FirmwareUpgraderClientStub());
#endif
}
