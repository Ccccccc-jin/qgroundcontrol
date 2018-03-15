#ifndef FIRMWAREUPDATECONFIG_H
#define FIRMWAREUPDATECONFIG_H

#include <QtCore>
#include <iostream>

class FirmwareUpdateSettings {
public:
    FirmwareUpdateSettings()
        : _checksumEnabled(true),
          _defaultFwSearchPath(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)),
          _defaultFwSavePath(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation))
    { fetch(); }

    ~FirmwareUpdateSettings(void) {
        sync();
    }

    void setChecksumEnabled(bool value) {
        _checksumEnabled = value;
    }

    void setDefaultFirmwareSearchPath(QString const& path) {
        _defaultFwSearchPath = path;
    }

    void setDefaultFirmwareSavePath(QString const& path) {
        _defaultFwSavePath = path;
    }

    bool checksumEnabeld(void) const { return _checksumEnabled; }
    QString const& defaultFirmwareSearchPath(void) const { return _defaultFwSearchPath; }
    QString const& defaultFirmwareSavePath(void)   const { return _defaultFwSavePath; }

    void sync(void) {
        QSettings appSettings;
        appSettings.beginGroup(_fwUpdateSettingsGroupName);
        appSettings.setValue(_checksumEnabledSetting, _checksumEnabled);
        appSettings.setValue(_defaultFwSearchPathSetting, _defaultFwSearchPath);
        appSettings.setValue(_defaultFwSavePathSetting, _defaultFwSavePath);
        appSettings.endGroup();
        appSettings.sync();
    }

    void fetch(void) {
        QSettings appSettings;
        appSettings.beginGroup(_fwUpdateSettingsGroupName);
        _fetchField<bool>(appSettings, _checksumEnabledSetting, _checksumEnabled);
        _fetchField<QString>(appSettings, _defaultFwSavePathSetting, _defaultFwSavePath);
        _fetchField<QString>(appSettings, _defaultFwSearchPathSetting, _defaultFwSearchPath);
        appSettings.endGroup();
    }

private:
    template<typename T>
    void _fetchField(QSettings const& settings, QString const fieldname, T& destField) {
        if (settings.contains(_checksumEnabledSetting)) {
            destField = qvariant_cast<T>(settings.value(fieldname));
        }
    }

    bool _checksumEnabled;
    QString _defaultFwSearchPath;
    QString _defaultFwSavePath;

    static constexpr auto _fwUpdateSettingsGroupName = "FirmwareUpdate";
    static constexpr auto _checksumEnabledSetting = "checksumEnabled";
    static constexpr auto _defaultFwSearchPathSetting = "firmwareSearchPath";
    static constexpr auto _defaultFwSavePathSetting = "firmwareSavePath";
};

#endif // FIRMWAREUPDATECONFIG_H
