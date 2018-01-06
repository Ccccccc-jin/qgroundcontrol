#!/bin/sh
HERE="$(dirname "$(readlink -f "${0}")")"
export LD_LIBRARY_PATH="${HERE}/usr/lib/x86_64-linux-gnu":"${HERE}/Qt/libs":$LD_LIBRARY_PATH
export QML2_IMPORT_PATH="${HERE}/Qt/qml"
export QT_PLUGIN_PATH="${HERE}/Qt/plugins"
mkdir -p ~/.icons && cp ${HERE}/qgroundcontrol.png ~/.icons

echo "App image path: $APPIMAGE"

if [ "$1" = "--fwupg" ]; then
    echo "start firmware upgrader"
    "${HERE}/fwupgrader-start.sh"
else 
    echo "start qgroundcontrol"
	"${HERE}/QGroundControl"
fi
