#include "FirmwareUpgraderInterface.h"
#if defined(Q_OS_WIN) || defined(Q_OS_UNIX)
#include "upgraders/FirmwareUpgraderClient.h"
#else
#include "upgraders/FirmwareUpgraderClientStub.h"
#endif



FirmwareUpgrader::FirmwareUpgrader(QObject* parent)
    : QObject(parent)
{  }


FirmwareUpgrader::~FirmwareUpgrader(void)
{  }


std::unique_ptr<FirmwareUpgrader> FirmwareUpgrader::instance() {
#if defined(Q_OS_UNIX) || defined(Q_OS_WIN)
    return std::unique_ptr<FirmwareUpgrader>(new FirmwareUpgraderClient());
#else
    return std::unique_ptr<FirmwareUpgrader>(new FirmwareUpgraderClientStub());
#endif
}
