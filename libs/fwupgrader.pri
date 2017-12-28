FW_UPGRADER_PROJECT = $$PWD/firmwareupgrader
FW_UPGRADER_PRO = $${FW_UPGRADER_PROJECT}/firmwareupgrader.pro
FW_UPGRADER_EXECUTABLE_NAME = fwupgrader

WindowsBuild {
    FW_UPGRADER_EXECUTABLE_NAME = \
        $${FW_UPGRADER_EXECUTABLE_NAME}.exe
}


exists($$shell_path($$FW_UPGRADER_PRO)) {
   BUILD_TYPE=""

   DebugBuild {
       BUILD_TYPE = "debug"
   } else {
       BUILD_TYPE = "release"
   }

   FIRMWARE_UPGRADER_BUILD_DIR_NAME = \
       $$shell_path($$absolute_path($$DESTDIR/../../build-firmwareupgrader-Qt-$${QT_VERSION}-$${BUILD_TYPE}))

   # First. we need to make firmware upgrader build dir on the same level
   # with qground. If builddir exists, we do not make it.

   MK_BUILDDIR_TARGET_NAME = $${FIRMWARE_UPGRADER_BUILD_DIR_NAME}
   mk_builddir.target      = $$MK_BUILDDIR_TARGET_NAME
   mk_builddir.commands    = \
       @echo "Make dir: $${MK_BUILDDIR_TARGET_NAME}"\
       && $$sprintf($$QMAKE_MKDIR_CMD, $${MK_BUILDDIR_TARGET_NAME})

   # Second. We need to build firmware upgrader in the previously made build dir.
   # If firware upgrader was built, we skip this step. (But each build we run qmake)

   BUILD_FWUPG_TARGET_NAME = \
      $$shell_path($${FIRMWARE_UPGRADER_BUILD_DIR_NAME}/$${BUILD_TYPE}/$${FW_UPGRADER_EXECUTABLE_NAME})

   FWUPG_DEPENDS_DIRS = \
      $$files($$shell_path($${FW_UPGRADER_PROJECT}/firmwareupgrader.pro)) \
      $$files($$shell_path($${FW_UPGRADER_PROJECT}/libs/*)) \
      $$files($$shell_path($${FW_UPGRADER_PROJECT}/src/*)) \
      $$files($$shell_path($${FW_UPGRADER_PROJECT}/src/devlib/*)) \
      $$files($$shell_path($${FW_UPGRADER_PROJECT}/src/components/*)) \
      $$files($$shell_path($${FW_UPGRADER_PROJECT}/src/util/*)) \
      $$files($$shell_path($${FW_UPGRADER_PROJECT}/src/usb/*)) \

   build_fwupg.target      = $$BUILD_FWUPG_TARGET_NAME
   build_fwupg.depends     = mk_builddir $${FWUPG_DEPENDS_DIRS}
   build_fwupg.commands    = \
       @echo "Build firmware upgrader" \
       && cd            $$shell_path($$FIRMWARE_UPGRADER_BUILD_DIR_NAME) \
       && $$QMAKE_QMAKE $$shell_path($$FW_UPGRADER_PRO) CONFIG+=$$BUILD_TYPE CONFIG+=NO_LIBS \
       && $(MAKE)

   QGC_BIN_DIR = $$DESTDIR

   # Third. We need to copy all files of firmware upgrader to qgc bin dir.
   # This target always need to be done.

   MOVE_BIN_TO_QGC_TARGET_NAME = \
      $$shell_path($$QGC_BIN_DIR/$${FW_UPGRADER_EXECUTABLE_NAME})

   move_bin_to_qgc.target      = $$MOVE_BIN_TO_QGC_TARGET_NAME
   move_bin_to_qgc.depends     = build_fwupg
   move_bin_to_qgc.commands    = \
       @echo "Move binary file to destionation dir in qground" \
       && $$QMAKE_COPY         $$shell_path($$FIRMWARE_UPGRADER_BUILD_DIR_NAME/$$BUILD_TYPE/*) \
                               $$shell_path($$QGC_BIN_DIR) \

   # For manual building from console. Usage: make upgrader

   build_fwupg_manual.target  = "upgrader"
   build_fwupg_manual.depends = move_bin_to_qgc

   QMAKE_EXTRA_TARGETS += mk_builddir build_fwupg move_bin_to_qgc build_fwupg_manual
   POST_TARGETDEPS     += $$MOVE_BIN_TO_QGC_TARGET_NAME
} else {
   error($$FW_UPGRADER_PRO doesn\'t exist)
}
