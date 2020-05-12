#ifndef TAIKO_TOOL_SETTINGSDIALOG_H
#define TAIKO_TOOL_SETTINGSDIALOG_H

#include <QDialog>
#include <QSettings>
#include <QFileDialog>
#include <QDebug>

#include "ui_SettingsDialog.h"

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QDialog *parent, QSettings *settings);
    ~SettingsDialog() = default;

private:
    Ui::SettingsDialog *ui;
};

#endif //TAIKO_TOOL_SETTINGSDIALOG_H
