#ifndef UPDATECONFIG_H
#define UPDATECONFIG_H

#include <QList>


class UpdateConfig
{
public:
    struct Vid  { int value; };
    struct Pids { QList<int> value; };
    struct ServerNodeName { QString value; };
    struct UpdBinaryPath { QString value; };

    UpdateConfig(Vid vid, Pids const& pids,
                 ServerNodeName const& serverNodeName,
                 UpdBinaryPath  const& fwUpdaterBinaryPath)
        : _vid(vid), _pids(pids),
          _serverNodeName(serverNodeName),
          _binaryPath(fwUpdaterBinaryPath)
    { }

    int        edgeVid()  const { return _vid.value; }
    QList<int> edgePids() const { return _pids.value; }
    QString    serverNodeName() const { return _serverNodeName.value; }
    QString    fwUpdaterBinaryPath() const { return _binaryPath.value; }

private:
    Vid   _vid;
    Pids  _pids;
    ServerNodeName _serverNodeName;
    UpdBinaryPath  _binaryPath;
};

#endif // UPDATECONFIG_H
