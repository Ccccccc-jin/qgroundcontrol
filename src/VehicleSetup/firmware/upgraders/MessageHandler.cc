#include <functional>

#include "rep_FirmwareUpgraderWatcher_replica.h"
#include "MessageHandler.h"
#include "States.h"


MessageHandler::MessageHandler(QObject *parent)
    : QObject(parent)
{  }


void MessageHandler::
    attach(std::shared_ptr<FirmwareUpgraderWatcherReplica> watcher)
{
    auto watcherPtr = watcher.get();

    using Watcher = FirmwareUpgraderWatcherReplica;
    using MsgHandler = MessageHandler;

    QObject::connect(watcherPtr, &Watcher::rpiBootStateChanged,
                     this,       &MsgHandler::onRpiBootStateChanged);

    QObject::connect(watcherPtr, &Watcher::deviceScannerStateChanged,
                     this,       &MsgHandler::onDeviceScannerStateChanged);

    QObject::connect(watcherPtr, &Watcher::flasherStateChanged,
                     this,       &MsgHandler::onFlasherStateChanged);

    qDebug() << "MessageHandler attached.";
}


void MessageHandler::onRpiBootStateChanged(uint state, uint type)
{
    auto rpiBootState = static_cast<states::RpiBootState>(state);
    auto messageSignal = _getMessageSignal(type);

    using RBState = states::RpiBootState;

    switch (rpiBootState) {
        case RBState::RpiBootDeviceFound:
            emit messageSignal("RpiBoot : device not found.");
            break;

        case RBState::RpiBootDeviceNotFound:
            emit messageSignal("RpiBoot : device found.");
            break;

        case RBState::RpiBootFailed:
            emit messageSignal("RpiBoot : failed.");
            break;

        case RBState::RpiBootFinished:
            emit messageSignal("RpiBoot : successfully finished.");
            break;

        case RBState::RpiBootStarted:
            emit messageSignal("RpiBoot : running...");
            break;

        default:
            emit errorMessageReceived("RpiBoot : undefined error.");
            break;
    }
}


void MessageHandler::onDeviceScannerStateChanged(uint state, uint type)
{
    auto deviceScannerState = static_cast<states::DeviceScannerState>(state);
    auto messageSignal = _getMessageSignal(type);

    using ScannerState = states::DeviceScannerState;

    switch (deviceScannerState) {
        case ScannerState::ScannerDeviceFound:
            emit messageSignal("DeviceScanner : device found.");
            break;

        case ScannerState::ScannerDeviceNotFound:
            emit messageSignal("DeviceScanner : device not found.");
            break;

        case ScannerState::ScannerStarted:
            emit messageSignal("DeviceScanner : running...");
            break;

        case ScannerState::ScannerFinished:
            emit messageSignal("DeviceScanner : successfully finished.");
            break;

        case ScannerState::ScannerFailed:
            emit messageSignal("DeviceScanner : failed.");
            break;

        default:
            emit errorMessageReceived("Undefined error.");
            break;
    }
}


void MessageHandler::onFlasherStateChanged(uint state, uint type)
{
    auto flasherState = static_cast<states::FlasherState>(state);
    auto messageSignal = _getMessageSignal(type);

    using FlasherState = states::FlasherState;

    switch (flasherState) {
        case FlasherState::FlasherStarted:
            emit messageSignal("Flasher : running... ");
            break;

        case FlasherState::FlasherOpenDeviceFailed:
            emit messageSignal("Flasher : open device file failed.");
            break;

        case FlasherState::FlasherOpenImageFailed:
            emit messageSignal("Flasher : open image file failed.");
            break;

        case FlasherState::FlasherFinished:
            emit messageSignal("Flasher : successfully finished.");
            break;

        case FlasherState::FlasherFailed:
            emit messageSignal("Flasher : failed.");
            break;

        case FlasherState::FlasherImageReadingFailed:
            emit messageSignal("Flasher : image reading failed.");
            break;

        case FlasherState::FlasherDeviceWritingFailed:
            emit messageSignal("Flasher : writing to device failed.");
            break;

        case FlasherState::FlasherCheckingCorrectnessStarted:
            emit messageSignal("Flasher : checking correctness, compute checksum...");
            break;

        case FlasherState::FlasherImageCorrectlyWrote:
            emit messageSignal("Flasher : image correctly wrote.");
            break;

        case FlasherState::FlasherImageUncorrectlyWrote:
            emit messageSignal("Flasher : image uncorretly wrote.");
            break;

        default:
            emit errorMessageReceived("Undefined error.");
            break;
    }
}


MessageSignal_t
    MessageHandler::_getMessageSignal(uint messageType)
{
    auto stateType = static_cast<states::StateType>(messageType);

    if (stateType == states::StateType::Info) {
        return [this] (QString const& msg) { emit infoMessageReceived(msg); };

    } else if (stateType == states::StateType::Warning) {
        return [this] (QString const& msg) { emit warnMessageReceived(msg); };

    } else {
        return [this] (QString const& msg) { emit errorMessageReceived(msg); };

    }
}
