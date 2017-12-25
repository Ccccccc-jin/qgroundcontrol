#include <functional>

#include "MessageHandler.h"
#include "States.h"


MessageHandler::MessageHandler(QObject *parent)
    : QObject(parent)
{  }


void MessageHandler::
    attach(std::shared_ptr<EdgeFirmwareUpdaterIPCReplica> watcher)
{
    auto watcherPtr = watcher.get();

    using Watcher = EdgeFirmwareUpdaterIPCReplica;
    using MsgHandler = MessageHandler;

    QObject::connect(watcherPtr, &Watcher::rpiBootStateChanged,
                     this,       &MsgHandler::onRpiBootStateChanged);

    QObject::connect(watcherPtr, &Watcher::deviceScannerStateChanged,
                     this,       &MsgHandler::onDeviceScannerStateChanged);

    QObject::connect(watcherPtr, &Watcher::flasherStateChanged,
                     this,       &MsgHandler::onFlasherStateChanged);

    QObject::connect(watcherPtr, &Watcher::checkingCorrectnessStateChanged,
                     this,       &MsgHandler::onCheckingCorrectnessStateChange);

    qDebug() << "MessageHandler attached.";
}


void MessageHandler::onRpiBootStateChanged(uint state, uint type)
{
    auto rpiBootState = static_cast<states::RpiBootState>(state);
    auto messageSignal =  [this, type] (QString const& msg) {
        _getMessageSignal(type)(QString("RpiBoot : ") + msg);
    };

    using RBState = states::RpiBootState;

    switch (rpiBootState) {
        case RBState::RpiBootDeviceFound:
            emit messageSignal("device not found.");
            break;

        case RBState::RpiBootDeviceNotFound:
            emit messageSignal("device found.");
            break;

        case RBState::RpiBootFailed:
            emit messageSignal("failed.");
            break;

        case RBState::RpiBootFinished:
            emit messageSignal("successfully finished.");
            break;

        case RBState::RpiBootStarted:
            emit messageSignal("running... please wait...");
            break;

        case RBState::RpiBootCancelled:
            emit messageSignal("cancelled.");
            break;

        default:
            emit errorMessageReceived("RpiBoot : undefined error.");
            break;
    }
}


void MessageHandler::onDeviceScannerStateChanged(uint state, uint type)
{
    auto deviceScannerState = static_cast<states::DeviceScannerState>(state);
    auto messageSignal =  [this, type] (QString const& msg) {
        _getMessageSignal(type)(QString("DeviceScanner : ") + msg);
    };

    using ScannerState = states::DeviceScannerState;

    switch (deviceScannerState) {
        case ScannerState::ScannerDeviceFound:
            emit messageSignal("device found.");
            break;

        case ScannerState::ScannerDeviceNotFound:
            emit messageSignal("device not found.");
            break;

        case ScannerState::ScannerStarted:
            emit messageSignal("running... please wait...");
            break;

        case ScannerState::ScannerFinished:
            emit messageSignal("successfully finished.");
            break;

        case ScannerState::ScannerFailed:
            emit messageSignal("failed.");
            break;

        case ScannerState::ScannerCancelled:
            emit messageSignal("cancelled.");
            break;

        default:
            emit errorMessageReceived("Undefined error.");
            break;
    }
}


void MessageHandler::onFlasherStateChanged(uint state, uint type)
{
    auto flasherState = static_cast<states::FlasherState>(state);
    auto messageSignal =  [this, type] (QString const& msg) {
        _getMessageSignal(type)(QString("Flasher : ") + msg);
    };

    using FlasherState = states::FlasherState;

    switch (flasherState) {
        case FlasherState::FlasherStarted:
            emit messageSignal("running... please wait...");
            break;

        case FlasherState::FlasherOpenDeviceFailed:
            emit messageSignal("open device file failed.");
            break;

        case FlasherState::FlasherOpenImageFailed:
            emit messageSignal("open image file failed.");
            break;

        case FlasherState::FlasherFinished:
            emit messageSignal("successfully finished.");
            break;

        case FlasherState::FlasherFailed:
            emit messageSignal("failed.");
            break;

        case FlasherState::FlasherImageReadingFailed:
            emit messageSignal("image reading failed.");
            break;

        case FlasherState::FlasherDeviceWritingFailed:
            emit messageSignal("writing to device failed.");
            break;

        case FlasherState::FlasherCancelled:
            emit messageSignal("cancelled.");
            break;

        default:
            emit errorMessageReceived("Undefined error.");
            break;
    }
}


void MessageHandler::onCheckingCorrectnessStateChange(uint state, uint type)
{
    auto checksumCalcState = static_cast<states::CheckingCorrectnessState>(state);
    auto messageSignal =  [this, type] (QString const& msg) {
        _getMessageSignal(type)(QString("Checksum : ") + msg);
    };

    using Checksum = states::CheckingCorrectnessState;

    switch (checksumCalcState) {
        case Checksum::CheckingCorrectnessStarted:
            emit messageSignal("running... please wait...");
            break;

        case Checksum::ComputeDeviceChecksum:
            emit messageSignal("compute device checksum...");
            break;

        case Checksum::ComputeImageChecksum:
            emit messageSignal("compute image checksum....");
            break;

        case Checksum::ReadingFailed:
            emit messageSignal("reading failed");
            break;

        case Checksum::ImageCorrectlyWrote:
            emit messageSignal("image correctly wrote.");
            break;

        case Checksum::ImageUncorrectlyWrote:
            emit messageSignal("image uncorrectly wrote.");
            break;

        case Checksum::CheckingCancelled:
            emit messageSignal("cancelled.");
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
