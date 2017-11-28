#include "ProcessStateLog.h"
#include <QtCore>

ProcessStateLog::ProcessStateLog(QObject *parent)
    : QObject(parent)
{  }


void ProcessStateLog::attach(const QProcess &process)
{
    QObject::connect(&process, &QProcess::stateChanged,
                     this,     &ProcessStateLog::onProcessStateChanged);

    QObject::connect(&process, &QProcess::errorOccurred,
                     this,     &ProcessStateLog::onProcessErrorOcurred);

    qInfo() << "ProcessStateLog: attached.";
}


void ProcessStateLog::onProcessErrorOcurred(QProcess::ProcessError error)
{
    using ProcError = QProcess::ProcessError;

    switch (error) {
        case ProcError::FailedToStart:
            qCritical() << "Process failed to start.";
            break;

        case ProcError::Timedout:
            qCritical() << "Process: timedout.";
            break;

        case ProcError::Crashed:
            qCritical() << "Process: crashed .";
            break;

        case ProcError::UnknownError:
            qCritical() << "Process: unknow error.";
            break;

        default:
            qFatal("Undefined error");
            std::abort();
            break;
    }
}


void ProcessStateLog::onProcessStateChanged(QProcess::ProcessState state)
{
    using ProcState = QProcess::ProcessState;

    switch (state) {
        case ProcState::Starting:
            qInfo() << "Process: starting";
            break;
        case ProcState::Running:
            qInfo() << "Process: running";
            break;
        case ProcState::NotRunning:
            qInfo() << "Process: not running";
            break;
        default:
            qFatal("Undefined error.");
            std::abort();
            break;
    }
}
