#ifndef PROCESSSTATELOG_H
#define PROCESSSTATELOG_H

#include <QProcess>

class ProcessStateLog : public QObject
{
    Q_OBJECT
public:
    explicit ProcessStateLog(QObject *parent = nullptr);

    void attach(QProcess const& process);

public slots:
    void onProcessStateChanged(QProcess::ProcessState state);

    void onProcessErrorOcurred(QProcess::ProcessError error);

};

#endif // PROCESSSTATELOG_H
