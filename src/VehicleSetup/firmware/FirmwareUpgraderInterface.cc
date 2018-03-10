#include "FirmwareUpgraderInterface.h"
#if defined(Q_OS_WIN) || defined(Q_OS_LINUX) || defined(Q_OS_MACX)
#include "upgraders/FirmwareUpgraderClient.h"
#else
#include "upgraders/FirmwareUpgraderClientStub.h"
#endif

#include "UpdateConfig.h"


FirmwareUpgrader::FirmwareUpgrader(QObject* parent)
    : QObject(parent)
{  }


FirmwareUpgrader::~FirmwareUpgrader(void)
{

}


std::unique_ptr<FirmwareUpgrader> FirmwareUpgrader::instance() {
    using Conf = UpdateConfig;

#if defined(Q_OS_LINUX) || defined(Q_OS_WIN) || defined(Q_OS_MACX)
    auto binaryPath = QCoreApplication::applicationDirPath() + "/fwupgrader";

    #ifdef Q_OS_MACX
        auto serverNodeName = "local:/tmp/fwupg_socket";
    #else
        auto serverNodeName = "local:fwupg_socket";
    #endif

    #ifdef Q_OS_WIN
        binaryPath.append(".exe").replace("/", "\\");
    #endif

    auto config = UpdateConfig(
                Conf::Vid{0x0a5c},
                Conf::Pids{QList<int>{0x2763, 0x2764, 0x0001}},
                Conf::ServerNodeName{serverNodeName},
                Conf::UpdBinaryPath{binaryPath}
    );

    return std::unique_ptr<FirmwareUpgrader>(new FirmwareUpgraderClient(config));
#else
    return std::unique_ptr<FirmwareUpgrader>(new FirmwareUpgraderClientStub());
#endif
}
