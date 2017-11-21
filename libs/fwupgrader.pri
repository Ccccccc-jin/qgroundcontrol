FW_UPGRADER_PRO = $$PWD/firmwareupgrader/firmwareupgrader.pro

exists($$FW_UPGRADER_PRO) {
   debug {
       BUILD_TYPE = "debug"
   } else {
       BUILD_TYPE = "release"
   }

   FIRMWARE_UPGRADER_BUILD_DIR_NAME = \
       build-firmwareupgrader-Qt-$${QT_VERSION}-$${BUILD_TYPE}

   build_fwupgrader.target   = build_firmware_upgrader
   build_fwupgrader.commands = \
       @echo "Build firmware upgrader" \
       && $$QMAKE_MKDIR ../$$FIRMWARE_UPGRADER_BUILD_DIR_NAME \
       && cd            ../$$FIRMWARE_UPGRADER_BUILD_DIR_NAME \
       && $$QMAKE_QMAKE $$FW_UPGRADER_PRO CONFIG+=$$BUILD_TYPE  \
       && $(MAKE)

   QGC_BIN_DIR = $$DESTDIR/bin

   move_to_qgc.target   = move_to_qgc
   move_to_qgc.commands = \
       @echo "Moving to qground" \
       && $$QMAKE_MKDIR $$QGC_BIN_DIR \
       && $$QMAKE_COPY  ../$$FIRMWARE_UPGRADER_BUILD_DIR_NAME/$$BUILD_TYPE/fwupgrader $$QGC_BIN_DIR/

   move_to_qgc.depends = build_fwupgrader

   QMAKE_EXTRA_TARGETS += build_fwupgrader move_to_qgc
   POST_TARGETDEPS     += move_to_qgc
} else {
   error($$FW_UPGRADER_PRO doesn\'t exist)
}
