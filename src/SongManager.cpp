#include <SongManager.h>

SongManager::SongManager(QSettings *settings) : settings(settings) {}

void SongManager::processGameDir(const QDir &dir, const QString &source, bool loadSystemWords) {
    QProgressDialog dialog("Loading songs...", "Cancel", 0, 1, nullptr);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.show();

    QFile musicInfoFile(dir.path() + "/datatable/musicinfo.bin");
    QFile wordListFile(dir.path() + "/datatable/wordlist.bin");

    if (!musicInfoFile.exists() || !wordListFile.exists())
        throw std::invalid_argument("Invalid game dir");

    musicInfoFile.open(QIODevice::ReadOnly);
    wordListFile.open(QIODevice::ReadOnly);

    QString musicInfoJson = QString::fromStdString(GZip::decompress(musicInfoFile.readAll().toStdString()));
    QString wordListJson = QString::fromStdString(GZip::decompress(wordListFile.readAll().toStdString()));
    QJsonDocument musicInfoDocument = QJsonDocument::fromJson(musicInfoJson.toUtf8());
    QJsonDocument wordListDocument = QJsonDocument::fromJson(wordListJson.toUtf8());
    QJsonArray items = musicInfoDocument["items"].toArray();
    QJsonArray wordList = wordListDocument["items"].toArray();
    SongDuplicateDialog::Intent intent = SongDuplicateDialog::Intent::NO;

    gameDirs << dir;

    dialog.setLabelText("Reading wordlist...");
    dialog.setMaximum(items.size() + wordList.size());

    for (auto word : wordList) {
        QString key = word.toObject()["key"].toString();
        int keyIndex = Utils::indexOfKeyValue(loadedWordList, "key", key);

        // Load word only if we want the system words (i.e. the base game wordlist that has menu text etc.) or it is a song entry
        if ((keyIndex < 0 && loadSystemWords) || (keyIndex < 0 && key.startsWith("song_")))
            loadedWordList << word;

        if (!dialog.wasCanceled()) {
            dialog.setValue(dialog.value() + 1);
            QCoreApplication::processEvents();
        } else
            return;
    }

    dialog.setLabelText("Loading songs...");

    for (const auto &item : items) {
        QString filePath = getSongPath(dir.path() + "/sound", item.toObject()["id"].toString());
        TaikoSong newSong(item.toObject()["id"].toString(), items, wordList, filePath);
        int dupeIndex = loadedSongs.indexOf(newSong);

        if (dupeIndex > -1) {
            if (intent == SongDuplicateDialog::Intent::NO_TO_ALL)
                goto update_progress;
            else if (intent == SongDuplicateDialog::Intent::YES_TO_ALL) {
                loadedSongs.removeAt(dupeIndex);
                goto load_song;
            }

            SongDuplicateDialog dupeDialog(nullptr, loadedSongs[dupeIndex]);
            dupeDialog.exec();
            intent = dupeDialog.getIntent();

            if (intent == SongDuplicateDialog::Intent::NO || intent == SongDuplicateDialog::Intent::NO_TO_ALL)
                goto update_progress;
            else
                loadedSongs.removeAt(dupeIndex);
        }

load_song:
        newSong.setTaikoToolSource(source);
        loadedSongs << newSong;

update_progress:
        if (!dialog.wasCanceled()) {
            dialog.setValue(dialog.value() + 1);
            QCoreApplication::processEvents();
        } else
            return;
    }
}

const QList<TaikoSong> &SongManager::getLoadedSongs() const {
    return loadedSongs;
}

TaikoSong &SongManager::getSongById(const QString &id) {
    for (auto &song : loadedSongs) {
        if (song.getId() == id)
            return song;
    }

    throw std::invalid_argument("Invalid ID.");
}

void SongManager::updateSongFlags(const QString &id, Flag flag, bool state) {
    switch (flag) {
        case SECRET:
            getSongById(id).setSecretFlag(state);
            break;
        case DLC:
            getSongById(id).setDLCFlag(state);
            break;
        case DEBUG:
            getSongById(id).setDebugFlag(state);
            break;
        case RECORDING:
            getSongById(id).setRecordingFlag(state);
    }

    emit songUpdated(id);
}

bool SongManager::stageSong(const QString &id) {
    if (Utils::indexOfString(stagedSongs, id) < 0) {
        // This song isn't already staged
        stagedSongs.append(id);
        return true;
    }

    return false;
}

void SongManager::stageSongs(const QStringList &ids) {
    int staged = 0;

    for (auto &id : ids)
        if (stageSong(id)) staged++;

    emit songsStaged(staged);
}

bool SongManager::isStaged(const QString &id) {
    return Utils::indexOfString(stagedSongs, id) > -1;
}

bool SongManager::unstageSong(const QString &id) {
    if (isStaged(id)) {
        stagedSongs.removeAt(Utils::indexOfString(stagedSongs, id));
        return true;
    }

    return false;
}

void SongManager::unstageSongs(const QStringList &ids) {
    int unstaged = 0;

    for (auto &id : ids)
        if (unstageSong(id)) unstaged++;

    emit songsUnstaged(unstaged);
}

const QStringList &SongManager::getStagedSongs() const {
    return stagedSongs;
}

QList<TaikoSong> SongManager::collectStagedSongs() {
    QList<TaikoSong> collectedSongs;
    QStringList songs = getStagedSongs();

    for (auto &song : songs)
        collectedSongs << getSongById(song);

    return collectedSongs;
}

QString SongManager::getSongPath(const QDir &dir, const QString &id) const {
    for (auto &fileName : dir.entryList(QStringList() << "*SONG_*", QDir::Files)) {
        QString foundId = fileName.toLower();
        foundId.remove(QRegExp(".*song_"));
        // chop off .nus3bank
        foundId.chop(9);

        if (foundId == id)
            return dir.filePath(fileName);
    }

    return QString();
}

void SongManager::build(const SongManager::BuildOptions &buildOptions) {
    QJsonObject musicInfoOutput;
    QJsonObject wordListObject;
    QJsonArray outputSongs;
    QJsonArray outputWordList;
    QList<TaikoSong> collectedSongs = collectStagedSongs();

    QProgressDialog dialog("Constructing wordlist...", "Cancel", 0, loadedWordList.size(), nullptr);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.show();

    // Modify any requested strings
    int newId = 0;
    int newUniqueId = 1;

    if (buildOptions.showVersionMainMenu || buildOptions.showVersionSongSelect) {
        int modeSelectIndex = Utils::indexOfKeyValue(loadedWordList, "key", "mode_select");
        int songSelectIndex = Utils::indexOfKeyValue(loadedWordList, "key", "select_song");

        if (modeSelectIndex > -1 && buildOptions.showVersionMainMenu) {
            QJsonObject obj = loadedWordList[modeSelectIndex].toObject();
            obj["englishUsText"] = "Select Mode. - (TaikoTool ver. 0.0.1)";
            obj["japaneseText"] = "モードをえらぶ - (TaikoTool ver. 0.0.1)";
            loadedWordList[modeSelectIndex] = obj;
        }

        if (songSelectIndex > -1 && buildOptions.showVersionSongSelect) {
            QJsonObject obj = loadedWordList[songSelectIndex].toObject();
            obj["englishUsText"] = "Select Song                                               (TT 0.0.1)";
            obj["japaneseText"] = "曲をえらぶ                                                      (TT 0.0.1)";
            loadedWordList[songSelectIndex] = obj;
        }
    }

    // Add all the other non-song entries to the wordList
    for (const auto &item : loadedWordList) {
        if (dialog.wasCanceled())
            return;

        dialog.setValue(dialog.value() + 1);
        QCoreApplication::processEvents();
        // Skip song entries as we already added them
        if (item.toObject()["key"].toString().contains(QRegExp(".*song_.*")))
            continue;

        outputWordList.append(item);
    }

    dialog.setValue(0);
    dialog.setMaximum(collectedSongs.size());
    dialog.setLabelText("Copying and converting songs if required- This may take some time.");
    QCoreApplication::processEvents();

    // Create output folder structure
    buildOptions.outDir.mkpath("./Data/NX/datatable");
    buildOptions.outDir.mkpath("./Data/NX/sound");
    buildOptions.outDir.mkpath("./Data/NX/fumen/enso");
    buildOptions.outDir.mkpath("./Data/NX/fumen_hitwide/enso");
    buildOptions.outDir.mkpath("./Data/NX/fumen_hitnarrow/enso");

    Queue<TaikoSong> songQueue;
    // Copy all songs and maps from all gameDirs to the build dir, but only if the song is staged
    for (auto &song : collectedSongs) {
        QString oldId;
        if (dialog.wasCanceled())
            return;

        if (buildOptions.showSourceSubText) {
            QString source = song.getTaikoToolSource();
            song.setSubTextEN(source + (source.isEmpty() ? "" : " - ") + song.getSubTextEN());
            song.setSubTextJP(source + (source.isEmpty() ? "" : " - ") + song.getSubTextJP());
        }

        oldId = song.getId();

        if (song.getTaikoToolSource() == "PS4")
            song.setId(QString::number(newId++).rightJustified(5, '0'));

        song.setUniqueId(newUniqueId++);

        outputWordList.append(song.getWordListSubEntry());
        outputWordList.append(song.getWordListEntry());
        outputSongs.append(song.getMusicInfoEntry());

        for (auto &dir : gameDirs) {
            QDir fumen(dir.path() + (song.getTaikoToolSource() == "PS4" ? "/fumen" : "/fumen/enso"));
            QDir fumenHitwide(dir.path() + (song.getTaikoToolSource() == "PS4" ? "/fumen_hitwide" : "/fumen_hitwide/enso"));
            QDir fumenHitnarrow(dir.path() + (song.getTaikoToolSource() == "PS4" ? "/fumen" : "/fumen_hitnarrow/enso"));
            QDir fumenOut(buildOptions.outDir.path() + "/Data/NX/fumen/enso/");
            QDir fumenHitwideOut(buildOptions.outDir.path() + "/Data/NX/fumen_hitwide/enso/");
            QDir fumenHitnarrowOut(buildOptions.outDir.path() + "/Data/NX/fumen_hitnarrow/enso/");

            // Copy all maps
            for (auto &dirName : fumen.entryList(QDir::Dirs)) {
                if (dirName != song.getId() && dirName != oldId)
                    continue;

                fumenOut.mkdir(song.getId());
                fumenHitwideOut.mkdir(song.getId());
                fumenHitnarrowOut.mkdir(song.getId());

                if (song.getTaikoToolSource() == "PS4") {
                    Utils::copyDir(fumen.path() + "/" + oldId, fumenOut.path() + "/" + song.getId(), oldId, song.getId());
                    Utils::copyDir(fumenHitwide.path() + "/" + oldId, fumenHitwideOut.path() + "/" + song.getId(), oldId, song.getId());
                    Utils::copyDir(fumenHitnarrow.path() + "/" + oldId, fumenHitnarrowOut.path() + "/" + song.getId(), oldId, song.getId());
                } else {
                    Utils::copyDir(fumen.path() + "/" + oldId, fumenOut.path() + "/" + song.getId());
                    Utils::copyDir(fumenHitwide.path() + "/" + oldId, fumenHitwideOut.path() + "/" + song.getId());
                    Utils::copyDir(fumenHitnarrow.path() + "/" + oldId, fumenHitnarrowOut.path() + "/" + song.getId());
                }
            }
        }

        // Copy song
        if (song.getTaikoToolSource() == "PS4") {
            QString outPath = buildOptions.outDir.path() + "/Data/NX/" + song.getSongFileName() + ".nus3bank";
            songQueue.enqueue(song);
//            Nus3bank::orbisToNX(settings, song.getSongFilePath(), outPath, song.getId());
        } else {
            QString fileName = song.getSongFileName().remove(0, 6) + ".nus3bank";
            QFile::copy(song.getSongFilePath(), buildOptions.outDir.path() + "/Data/NX/sound/" + fileName);
        }

        dialog.setValue(dialog.value() + 1);
        QCoreApplication::processEvents();
    }

    bool finished = false;
    QueueProcessor *queueProcessor(new QueueProcessor(&songQueue, 2));
    connect(queueProcessor, &QueueProcessor::progress, this, []() {
        qDebug() << "Progress";
    });

    connect(queueProcessor, &QueueProcessor::finished, this, [&finished]() {
        qDebug() << "finished";
        finished = true;
    });

    queueProcessor->start(settings, new QDir(buildOptions.outDir.path() + "/Data/NX/"));


    qDebug() << "Started queue processing";

    while(!finished)
        QCoreApplication::processEvents();

    delete queueProcessor;

    qDebug() << "Stopped";

    musicInfoOutput["items"] = outputSongs;
    wordListObject["items"] = outputWordList;

    QJsonDocument musicInfoDocument = QJsonDocument(musicInfoOutput);
    QJsonDocument wordListDocument = QJsonDocument(wordListObject);

    // Compress wordlist and musicinfo json
    gzFile musicInfoFile = gzopen(QString(buildOptions.outDir.path() + "/Data/NX/datatable/musicinfo.bin").toStdString().c_str(), "wb");
    gzputs(musicInfoFile, musicInfoDocument.toJson().toStdString().c_str());
    gzclose(musicInfoFile);

    gzFile wordListFile = gzopen(QString(buildOptions.outDir.path() + "/Data/NX/datatable/wordlist.bin").toStdString().c_str(), "wb");
    gzputs(wordListFile, wordListDocument.toJson().toStdString().c_str());
    gzclose(wordListFile);

    emit buildCompleted();
}
