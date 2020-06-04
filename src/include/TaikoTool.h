#ifndef TAIKO_TOOL_TAIKOTOOL_H
#define TAIKO_TOOL_TAIKOTOOL_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QtGlobal>
#include <QStandardPaths>
#include <QSettings>
#include <QProgressDialog>

#include <dialogs/SettingsDialog.h>
#include <SongManager.h>
#include <SongTabManager.h>
#include <BuildTabManager.h>
#include <GZip.h>
#include <Convert.h>

#include "ui_TaikoTool.h"

namespace Ui {
    class TaikoTool;
}

class TaikoTool : public QMainWindow {
    Q_OBJECT

public:
    explicit TaikoTool(QMainWindow *parent = nullptr);
    ~TaikoTool() = default;

private slots:
    void baseGameActionTriggered();
    void ps4GameActionTriggered();
    void settingsActionTriggered();

private:
    Ui::TaikoTool *ui;
    SongManager *songManager;
    SongTabManager *songTabManager;
    BuildTabManager *buildTabManager;
    QSettings *settings;
};


#endif //TAIKO_TOOL_TAIKOTOOL_H
