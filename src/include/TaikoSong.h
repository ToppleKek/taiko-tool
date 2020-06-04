#ifndef TAIKO_TOOL_TAIKOSONG_H
#define TAIKO_TOOL_TAIKOSONG_H

#include <QJsonArray>
#include <QJsonObject>
#include <stdexcept>

#include <Utils.h>

class TaikoSong {
public:
    struct TaikoSongDifficulties {
        int kantan;
        int futsuu;
        int muzukashii;
        int oni;
        int ura;
        bool branchKantan;
        bool branchFutsuu;
        bool branchMuzukashii;
        bool branchOni;
        bool branchUra;
    };

    struct TaikoSongFlags {
        bool secretFlag;
        bool dlc;
        bool debug;
        bool recording;
    };

    TaikoSong();
    TaikoSong(const QString &id, const QJsonArray &musicInfo, const QJsonArray &wordList, const QString &songFilePath);
    ~TaikoSong() = default;

    const QJsonObject &getMusicInfoEntry() const;
    const QJsonObject &getWordListEntry() const;
    const QJsonObject &getWordListSubEntry() const;
    QString getTitleEN() const;
    QString getTitleJP() const;
    QString getId() const;
    int getUniqueId() const;
    TaikoSongDifficulties getDifficulties() const;
    TaikoSongFlags getFlags() const;
    int getGenre() const;
    int getOrder() const;
    QString getSongFileName() const;
    QString getSubTextEN() const;
    QString getSubTextJP() const;
    const QString &getTaikoToolSource() const;
    const QString &getSongFilePath() const;

    void setId(const QString &id);
    void setUniqueId(int uniqueId);
    void setTitleEN(const QString &title);
    void setSubTextEN(const QString &subText);
    void setSubTextJP(const QString &subText);
    void setTaikoToolSource(const QString &source);
    void setDifficultyKantan(int stars);
    void setDifficultyFutsuu(int stars);
    void setDifficultyMuzukashii(int stars);
    void setDifficultyOni(int stars);
    void setDifficultyUra(int stars);
    void setSecretFlag(bool state);
    void setDLCFlag(bool state);
    void setDebugFlag(bool state);
    void setRecordingFlag(bool state);
    void setOrder(int order);

    bool operator==(const TaikoSong &rhs) const;
    bool operator!=(const TaikoSong &rhs) const;

private:
    QJsonObject musicInfoEntry;
    QJsonObject wordListEntry;
    QJsonObject wordListSubEntry;
    QString taikoToolSource;
    QString songFilePath;
};

#endif //TAIKO_TOOL_TAIKOSONG_H
