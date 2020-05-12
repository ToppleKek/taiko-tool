#include <TaikoSong.h>

TaikoSong::TaikoSong() : musicInfoEntry(), wordListEntry(), wordListSubEntry(), songFilePath(), taikoToolSource() {}

TaikoSong::TaikoSong(const QString &id, const QJsonArray &musicInfo, const QJsonArray &wordList,
                     const QString &songFilePath) : songFilePath(songFilePath) {
    int musicInfoIndex = Utils::indexOfKeyValue(musicInfo, "id", id);
    int wordListIndex = Utils::indexOfKeyValue(wordList, "key", "song_" + id);
    int wordListSubIndex = Utils::indexOfKeyValue(wordList, "key", "song_sub_" + id);

    if (musicInfoIndex < 0 || wordListIndex < 0)
        throw std::invalid_argument("The provided musicInfo or wordList array do not contain the id provided.");

    musicInfoEntry = musicInfo[musicInfoIndex].toObject();
    wordListEntry = wordList[wordListIndex].toObject();

    // Only the PS4 game has the "entitlementKey"
    if (musicInfoEntry.contains("entitlementKey")) {
        if (wordListSubIndex < 0) {
            wordListSubEntry["japaneseText"] = "";
            wordListSubEntry["englishUsText"] = "";
        } else
            wordListSubEntry = wordList[wordListSubIndex].toObject();

        int wordListDetailIndex = Utils::indexOfKeyValue(wordList, "key", "song_detail_" + id);
        QString wordListDetail = wordList[wordListDetailIndex].toObject()["englishUsText"].toString();

        // Replace Japanese name with English name because the PS4 game stores this in a "song_detail_*" key
        if (wordListDetailIndex > -1 && !wordListDetail.isEmpty())
            wordListEntry["englishUsText"] = wordListDetail;

        musicInfoEntry["secretFlag"] = false;
        musicInfoEntry["recording"] = true;
        musicInfoEntry["alleviationEasy"] = false;
        musicInfoEntry["alleviationNormal"] = false;
        musicInfoEntry["alleviationHard"] = false;
        musicInfoEntry["alleviationMania"] = false;
        musicInfoEntry["alleviationUra"] = false;

        musicInfoEntry.remove("entitlementKey");
    } else
        wordListSubEntry = wordList[wordListSubIndex].toObject();
}

const QJsonObject &TaikoSong::getMusicInfoEntry() const {
    return musicInfoEntry;
}

const QJsonObject &TaikoSong::getWordListEntry() const {
    return wordListEntry;
}

const QJsonObject &TaikoSong::getWordListSubEntry() const {
    return wordListSubEntry;
}

QString TaikoSong::getTitleEN() const {
    return wordListEntry["englishUsText"].toString();
}

QString TaikoSong::getTitleJP() const {
    return wordListEntry["japaneseText"].toString();
}

QString TaikoSong::getId() const {
    return musicInfoEntry["id"].toString();
}

int TaikoSong::getUniqueId() const {
    return musicInfoEntry["uniqueId"].toInt();
}

TaikoSong::TaikoSongDifficulties TaikoSong::getDifficulties() const {
    return TaikoSong::TaikoSongDifficulties{
            musicInfoEntry["starEasy"].toInt(),
            musicInfoEntry["starNormal"].toInt(),
            musicInfoEntry["starHard"].toInt(),
            musicInfoEntry["starMania"].toInt(),
            musicInfoEntry["starUra"].toInt(),
            musicInfoEntry["branchEasy"].toBool(),
            musicInfoEntry["branchNormal"].toBool(),
            musicInfoEntry["branchHard"].toBool(),
            musicInfoEntry["branchMania"].toBool(),
            musicInfoEntry["branchUra"].toBool()
    };
}

TaikoSong::TaikoSongFlags TaikoSong::getFlags() const {
    return TaikoSong::TaikoSongFlags{
            musicInfoEntry["secretFlag"].toBool(),
            musicInfoEntry["dlc"].toBool(),
            musicInfoEntry["debug"].toBool(),
            musicInfoEntry["recording"].toBool()
    };
}


int TaikoSong::getGenre() const {
    return musicInfoEntry["genreNo"].toInt();
}

int TaikoSong::getOrder() const {
    return musicInfoEntry["order"].toInt();
}

QString TaikoSong::getSongFileName() const {
    return musicInfoEntry["songFileName"].toString();
}

QString TaikoSong::getSubTextEN() const {
    return wordListSubEntry["englishUsText"].toString();
}

QString TaikoSong::getSubTextJP() const {
    return wordListSubEntry["japaneseText"].toString();
}

const QString &TaikoSong::getTaikoToolSource() const {
    return taikoToolSource;
}

const QString &TaikoSong::getSongFilePath() const {
    return songFilePath;
}

void TaikoSong::setId(const QString &id) {
    musicInfoEntry["id"] = id;
    musicInfoEntry["songFileName"] = "sound/SONG_" + id.toUpper();
    wordListEntry["key"] = "song_" + id;
    wordListSubEntry["key"] = "song_sub_" + id;
}

void TaikoSong::setUniqueId(int uniqueId) {
    musicInfoEntry["uniqueId"] = uniqueId;
}

void TaikoSong::setTitleEN(const QString &title) {
    wordListEntry["englishUsText"] = title;
}

void TaikoSong::setSubTextEN(const QString &subText) {
    wordListSubEntry["englishUsText"] = subText;
}

void TaikoSong::setSubTextJP(const QString &subText) {
    wordListSubEntry["japaneseText"] = subText;
}

void TaikoSong::setTaikoToolSource(const QString &source) {
    taikoToolSource = source;
}

void TaikoSong::setDifficultyKantan(int stars) {
    musicInfoEntry["starEasy"] = stars;
}

void TaikoSong::setDifficultyFutsuu(int stars) {
    musicInfoEntry["starNormal"] = stars;
}

void TaikoSong::setDifficultyMuzukashii(int stars) {
    musicInfoEntry["starHard"] = stars;
}

void TaikoSong::setDifficultyOni(int stars) {
    musicInfoEntry["starMania"] = stars;
}

void TaikoSong::setDifficultyUra(int stars) {
    musicInfoEntry["starUra"] = stars;
}

void TaikoSong::setSecretFlag(bool state) {
    musicInfoEntry["secretFlag"] = state;
}

void TaikoSong::setDLCFlag(bool state) {
    musicInfoEntry["dlc"] = state;
}

void TaikoSong::setDebugFlag(bool state) {
    musicInfoEntry["debug"] = state;
}

void TaikoSong::setRecordingFlag(bool state) {
    musicInfoEntry["recording"] = state;
}

bool TaikoSong::operator==(const TaikoSong &rhs) const {
    return getId() == rhs.getId();
}

bool TaikoSong::operator!=(const TaikoSong &rhs) const {
    return !(rhs == *this);
}
