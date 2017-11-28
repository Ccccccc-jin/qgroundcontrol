FW_UPGRADER_PRO = $$PWD/firmwareupgrader/firmwareupgrader.pro

exists($$shell_path($$FW_UPGRADER_PRO)) {
   BUILD_TYPE=""

   DebugBuild {
       BUILD_TYPE = "debug"
   }

   ReleaseBuild {
       BUILD_TYPE = "release"
   }

   FIRMWARE_UPGRADER_BUILD_DIR_NAME = \
       $$DESTDIR/../../build-firmwareupgrader-Qt-$${QT_VERSION}-$${BUILD_TYPE}

   # First. we need to make firmware upgrader build dir on the same level
   # with qground. If builddir exists, we do not make it.

   MK_BUILDIR_TARGET_NAME = $$shell_path($$FIRMWARE_UPGRADER_BUILD_DIR_NAME)
   mk_builddir.target     = $$MK_BUILDDIR_TARGET_NAME
   mk_builddir.commands   = \
       @echo "Make build dir of firmware upgrader" \
       && $$sprintf($$QMAKE_MKDIR_CMD, $$shell_path($$FIRMWARE_UPGRADER_BUILD_DIR_NAME))

   # Second. We need to build firmware upgrader in the previously made build dir.
   # If firware upgrader was built, we skip this step. (But each build we run qmake)

   BUILD_FWUPG_TARGET_NAME = build_fw_upgrader
   build_fwupg.target      = $$BUILD_FWUPG_TARGET_NAME
   build_fwupg.depends     = mk_builddir
   build_fwupg.commands    = \
       @echo "Build firmware upgrader" \
       && cd            $$shell_path($$FIRMWARE_UPGRADER_BUILD_DIR_NAME) \
       && $$QMAKE_QMAKE $$shell_path($$FW_UPGRADER_PRO) CONFIG+=$$BUILD_TYPE \
       && $(MAKE)

   QGC_BIN_DIR           = $$DESTDIR/bin

   # Third. We need to make 'bin' directory in qground DESTDIR.
   # If this dir exists, we skip this step.

   MK_BINDIR_TARGET_NAME = $$QGC_BIN_DIR
   mk_bindir.target      = $$MK_BINDIR_TARGET_NAME
   mk_bindir.commands    = \
       @echo "Make bin dir in qground destination dir." \
       && $$sprintf($$QMAKE_MKDIR_CMD, $$shell_path($$QGC_BIN_DIR))

   # Fourth. We need to copy all files of firmware upgrader to qgc bin dir.
   # This target always need to be done.

   MOVE_BIN_TO_QGC_TARGET_NAME = move_bin_to_qground
   move_bin_to_qgc.target      = $$MOVE_BIN_TO_QGC_TARGET_NAME
   move_bin_to_qgc.depends     = build_fwupg mk_bindir
   move_bin_to_qgc.commands    = \
       @echo "Move binary file to destionation dir in qground" \
       && $$QMAKE_COPY         $$shell_path($$FIRMWARE_UPGRADER_BUILD_DIR_NAME/$$BUILD_TYPE/*) \
                               $$shell_path($$QGC_BIN_DIR) \


   QMAKE_EXTRA_TARGETS = mk_builddir build_fwupg mk_bindir move_bin_to_qgc
   POST_TARGETDEPS += $$MOVE_BIN_TO_QGC_TARGET_NAME
} else {
   error($$FW_UPGRADER_PRO doesn\'t exist)
}
