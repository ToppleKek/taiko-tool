#ifndef TAIKO_TOOL_SONGMANAGER_H
#define TAIKO_TOOL_SONGMANAGER_H

#include <sstream>
#include <QString>
#include <QList>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QtGlobal>
#include <QDir>
#include <QDataStream>
#include <QProgressDialog>
#include <QCoreApplication>
#include <QSettings>
#include <QQueue>
#include <QtConcurrent>
#include <zlib.h>

#include <dialogs/OperationDialogs.h>
#include <GZip.h>
#include <Utils.h>
#include <TaikoSong.h>
#include <Nus3bank.h>
#include <Queue.h>

class SongManager : public QObject {
    Q_OBJECT

public:
    static constexpr const char *REQUIRED_JSON_KEYS[] = {
            "uniqueId",
            "id",
            "songFileName",
            "order",
            "genreNo",
            "secretFlag",
            "dlc",
            "debug",
            "recording",
            "branchEasy",
            "branchNormal",
            "branchHard",
            "branchMania",
            "branchUra",
            "starEasy",
            "starNormal",
            "starHard",
            "starMania",
            "starUra",
            "shinutiEasy",
            "shinutiNormal",
            "shinutiHard",
            "shinutiMania",
            "shinutiUra",
            "shinutiEasyDuet",
            "shinutiNormalDuet",
            "shinutiHardDuet",
            "shinutiManiaDuet",
            "shinutiUraDuet",
            "scoreEasy",
            "scoreNormal",
            "scoreHard",
            "scoreMania",
            "scoreUra",
            "alleviationEasy", // Whatever alleviation is (these are almost always false)
            "alleviationNormal",
            "alleviationHard",
            "alleviationMania",
            "alleviationUra"
            // NOTE: bgDon0, bgDancer0, chibi0, rendaEffect0, etc. seem to be unused: they will not be included
    };

    enum Flag {
        SECRET,
        DLC,
        DEBUG,
        RECORDING
    };

    struct BuildOptions {
        QDir outDir;
        bool showVersionMainMenu;
        bool showVersionSongSelect;
        bool showSourceSubText;
    };
    //TODO: singleton
    explicit SongManager(QSettings *settings);

    void processGameDir(const QDir &dir, const QString &source, bool loadSystemWords = false);
    TaikoSong &getSongById(const QString& id);
    const QList<TaikoSong> &getLoadedSongs() const;
    const QStringList &getStagedSongs() const;
    QList<TaikoSong> collectStagedSongs();
    QString getSongPath(const QDir &dir, const QString &id) const;
    void updateSongFlags(const QString &id, Flag flag, bool state);
    bool stageSong(const QString &id);
    void stageSongs(const QStringList &ids);
    bool unstageSong(const QString &id);
    void unstageSongs(const QStringList &ids);
    bool isStaged(const QString &id);
    void build(const BuildOptions &buildOptions);

private:
    QList<TaikoSong> loadedSongs;
    QJsonArray loadedWordList;
    QStringList stagedSongs;
    QList<QDir> gameDirs;
    QSettings *settings;

signals:
    void songUpdated(const QString &id);
    void songsStaged(int numberStaged);
    void songsUnstaged(int numberUnstaged);
    void buildCompleted();
};

#endif //TAIKO_TOOL_SONGMANAGER_H
