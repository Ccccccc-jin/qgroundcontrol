#ifndef EDGEMESSAGEHANDLER_H
#define EDGEMESSAGEHANDLER_H

#include <QObject>

class EdgeMessageHandler : public QObject
{
    Q_OBJECT
public:
    explicit EdgeMessageHandler(QObject *parent = nullptr);

signals:

public slots:
};

#endif // EDGEMESSAGEHANDLER_H