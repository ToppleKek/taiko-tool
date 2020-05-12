#ifndef TAIKO_TOOL_SONGTABMANAGER_H
#define TAIKO_TOOL_SONGTABMANAGER_H

#include <QObject>
#include <QTreeWidget>
#include <QJsonArray>
#include <QJsonObject>
#include <QMediaPlayer>
#include <QProcess>
#include <QStandardPaths>
#include <QDir>
#include <QButtonGroup>
#include <QSettings>
#include <QMessageBox>

#include <TaikoSong.h>
#include <SongManager.h>

#include "ui_TaikoTool.h"

#define STARS(stars) numberToStars(stars) + "  " + QString::number(stars)
#define DIVERGING_NOTES(a) QString(a ? " <img src=':/icons/diverging-notes.png' width=16 height=16 style='vertical-align:middle;'>" : "")
#define ICON_KANTAN QString("<img src=':/icons/diff_kantan_small.png' width=32 height=32 style='vertical-align:middle;'/> ")
#define ICON_FUTSUU QString("<br><img src=':/icons/diff_futsuu_small.png' width=32 height=32 style='vertical-align:middle;'/> ")
#define ICON_MUZUKASHII QString("<br><img src=':/icons/diff_muzukashii_small.png' width=32 height=32 style='vertical-align:middle;'/>")
#define ICON_ONI QString("<br><img src=':/icons/diff_oni_small.png' width=32 height=32 style='vertical-align:middle;'/>")
#define ICON_URA QString("<br><img src=':/icons/diff_ura_small.png' width=32 height=32 style='vertical-align:middle;'/>")
#define ID_COLUMN 4
#define MODIFIED_COLUMN 6

class SongTabManager : public QObject {
    Q_OBJECT

public:
    explicit SongTabManager(Ui::TaikoTool *ui, SongManager *songManager, QSettings *settings);
    ~SongTabManager() = default;
    void updateTree(const QList<TaikoSong> &items);

private slots:
    void selectionChanged();
    void playStopButtonClicked();
    void singleSongPropCheckboxToggled(QAbstractButton *button, bool checked);
    void multiSongPropButtonClicked(int id);
    void songUpdated(const QString &id);
    void songTreeContextMenu(const QPoint &pos);
    void addToBuildButtonClicked();
    void removeFromBuildButtonClicked();

private:
    Ui::TaikoTool *ui;
    SongManager *songManager;
    QMediaPlayer *mediaPlayer;
    QSettings *settings;

    static QString numberToStars(int stars);

    static constexpr const char *GENRE_MAPPING[] = {
        "Pop",
        "Anime",
        "Vocaloid",
        "Variety",
        "Unknown",
        "Classical",
        "Game",
        "Namco Original"
    };
};

#endif //TAIKO_TOOL_SONGTABMANAGER_H
