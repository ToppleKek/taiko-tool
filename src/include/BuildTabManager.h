#ifndef TAIKO_TOOL_BUILDTABMANAGER_H
#define TAIKO_TOOL_BUILDTABMANAGER_H

#include <QObject>
#include <QFileDialog>
#include <SongManager.h>

#include "ui_TaikoTool.h"

class BuildTabManager : public QObject {
    Q_OBJECT

public:
    explicit BuildTabManager(Ui::TaikoTool *ui, SongManager *songManager);
    ~BuildTabManager() = default;

    void updateList();

private slots:
    void songsStaged(int numberStaged);
    void buildButtonClicked();

private:
    SongManager *songManager;
    Ui::TaikoTool *ui;
};


#endif //TAIKO_TOOL_BUILDTABMANAGER_H
