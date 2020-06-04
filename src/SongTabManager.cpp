#include <SongTabManager.h>

SongTabManager::SongTabManager(Ui::TaikoTool *ui, SongManager *songManager, QSettings *settings) :
                               ui(ui), songManager(songManager), mediaPlayer(new QMediaPlayer), settings(settings) {
    ui->playStopButton->setVisible(false);
    ui->songPropertiesStackedWidget->setVisible(false);

    connect(ui->songTree, &QTreeWidget::itemSelectionChanged, this, &SongTabManager::selectionChanged);
    connect(ui->playStopButton, &QPushButton::clicked, this, &SongTabManager::playStopButtonClicked);
    connect(ui->addToBuildButton, &QPushButton::clicked, this, &SongTabManager::addToBuildButtonClicked);
    connect(ui->removeFromBuildButton, &QPushButton::clicked, this, &SongTabManager::removeFromBuildButtonClicked);
    connect(ui->singleSongPropButtonGroup, QOverload<QAbstractButton *, bool>::of(&QButtonGroup::buttonToggled), this, &SongTabManager::singleSongPropCheckboxToggled);
    connect(ui->multiSongPropButtonGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), this, &SongTabManager::multiSongPropButtonClicked);
    connect(songManager, &SongManager::songUpdated, this, &SongTabManager::songUpdated);
    connect(ui->songTree, &QWidget::customContextMenuRequested, this, &SongTabManager::songTreeContextMenu);

    connect(mediaPlayer, &QMediaPlayer::stateChanged, this, [ui](QMediaPlayer::State state) {
        if (state == QMediaPlayer::StoppedState)
            ui->playStopButton->setText("\u25B6");
        else if (state == QMediaPlayer::PlayingState)
            ui->playStopButton->setText("\u25A0");
    });

    connect(ui->songTabSplitter, &QSplitter::splitterMoved, this, [=](int pos, int index) {
        QList<QTreeWidgetItem *> items = ui->songTree->selectedItems();
        TaikoSong song = songManager->getSongById(items.first()->text(ID_COLUMN));

        ui->songNameENLabel->setText(ui->songNameENLabel->fontMetrics().elidedText(song.getTitleEN(),
                                                                                   Qt::ElideRight, ui->songNameENLabel->width()));

        ui->songNameJPLabel->setText(ui->songNameJPLabel->fontMetrics().elidedText(song.getTitleJP(),
                                                                                   Qt::ElideRight, ui->songNameJPLabel->width()));

        ui->songSubLabel->setText(ui->songSubLabel->fontMetrics().elidedText(song.getSubTextEN(),
                                                                             Qt::ElideRight, ui->songSubLabel->width()));
    });
}

void SongTabManager::updateTree(const QList<TaikoSong> &items) {
    for (const auto &item : items) {
        QList<QTreeWidgetItem *> dupes = ui->songTree->findItems(item.getId(), Qt::MatchExactly, ID_COLUMN);

        if (!dupes.empty())
            delete dupes.first();

        ui->songTree->addTopLevelItem(new QTreeWidgetItem(QStringList()
                                      << item.getTaikoToolSource() << item.getTitleEN() << GENRE_MAPPING[item.getGenre()]
                                      << QString::number(item.getOrder()) << item.getId() << QString::number(item.getUniqueId())));
    }
}

void SongTabManager::selectionChanged() {
    QList<QTreeWidgetItem *> items = ui->songTree->selectedItems();
    QString id;

    if (mediaPlayer->state() == QMediaPlayer::PlayingState)
        mediaPlayer->stop();

    if (items.size() > 1) {
        ui->songNameJPLabel->setText(QString::number(items.size()) + " songs selected");
        ui->songPropertiesStackedWidget->setVisible(true);
        ui->songPropertiesStackedWidget->setCurrentIndex(1);
        ui->buildButtonWidget->setVisible(true);
        goto hide;
    } else if (items.empty()) {
        ui->songNameJPLabel->setText("Select a song");
        ui->songPropertiesStackedWidget->setVisible(false);
        ui->buildButtonWidget->setVisible(false);
        goto hide;
    } else {
        id = items.first()->text(ID_COLUMN);
        TaikoSong song = songManager->getSongById(id);
        TaikoSong::TaikoSongDifficulties difficulties = song.getDifficulties();
        TaikoSong::TaikoSongFlags flags = song.getFlags();

        ui->songNameENLabel->setText(ui->songNameENLabel->fontMetrics().elidedText(song.getTitleEN(),
                                     Qt::ElideRight, ui->songNameENLabel->width()));

        ui->songNameJPLabel->setText(ui->songNameJPLabel->fontMetrics().elidedText(song.getTitleJP(),
                                     Qt::ElideRight, ui->songNameJPLabel->width()));

        ui->songSubLabel->setText(ui->songSubLabel->fontMetrics().elidedText(song.getSubTextEN(),
                                  Qt::ElideRight, ui->songSubLabel->width()));

        ui->playStopButton->setVisible(true);
        ui->buildButtonWidget->setVisible(true);
        ui->songNameENLabel->setVisible(true);
        ui->songSubLabel->setVisible(true);
        ui->difficultiesLabel->setVisible(true);

        ui->difficultiesLabel->setText(
                ICON_KANTAN + STARS(difficulties.kantan) + DIVERGING_NOTES(difficulties.branchKantan) +
                ICON_FUTSUU + STARS(difficulties.futsuu) + DIVERGING_NOTES(difficulties.branchFutsuu) +
                ICON_MUZUKASHII + STARS(difficulties.muzukashii) + DIVERGING_NOTES(difficulties.branchMuzukashii) +
                ICON_ONI + STARS(difficulties.oni) + DIVERGING_NOTES(difficulties.branchOni) +
                (difficulties.ura == 0 ? "" : ICON_URA + STARS(difficulties.ura) +
                                              DIVERGING_NOTES(difficulties.branchUra)));

        ui->singleSongPropButtonGroup->blockSignals(true);

        ui->songSecretCheckbox->setChecked(flags.secretFlag);
        ui->songDLCCheckbox->setChecked(flags.dlc);
        ui->songDebugCheckbox->setChecked(flags.debug);
        ui->songRecordableCheckbox->setChecked(flags.recording);

        ui->singleSongPropButtonGroup->blockSignals(false);

        ui->songPropertiesStackedWidget->setVisible(true);
        ui->songPropertiesStackedWidget->setCurrentIndex(0);
    }

    return;

hide:
    ui->songNameENLabel->setVisible(false);
    ui->songSubLabel->setVisible(false);
    ui->difficultiesLabel->setVisible(false);
    ui->playStopButton->setVisible(false);
}

void SongTabManager::playStopButtonClicked() {
    if (mediaPlayer->state() == QMediaPlayer::PlayingState) {
        mediaPlayer->stop();
        return;
    }

    if (!settings->contains("vgmstream/path")) {
        QMessageBox::critical(nullptr, "Vgmstream path not set",
                              "The path to the vgmstream-cli binary was not set. Please set the path to the vgmstream-cli binary to enable previews!");
        return;
    }

    if (!QFile(settings->value("vgmstream/path").toString()).exists()) {
        QMessageBox::critical(nullptr, "Vgmstream not found",
                              "Cannot find vgmstream-cli binary. Check that the path is correct and the file is present.");
        return;
    }

    QString cache = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QString vgmstreamPath = settings->value("vgmstream/path").toString();
    QString songFilePath = songManager->getSongById(
            ui->songTree->selectedItems().first()->text(ID_COLUMN)).getSongFilePath();

    QProcess::execute(vgmstreamPath, QStringList() << "-o" << cache + "/sound/preview.wav" << songFilePath);

    mediaPlayer->setMedia(QUrl::fromLocalFile(cache + "/sound/preview.wav"));
    mediaPlayer->setVolume(100);
    mediaPlayer->play();
}

QString SongTabManager::numberToStars(int stars) {
    QString str;

    for (int i = 0; i < 10; i++)
        str += i < stars ? "\u2605" : "\u2606";

    return str;
}

void SongTabManager::singleSongPropCheckboxToggled(QAbstractButton *button, bool checked) {
    QString id = ui->songTree->selectedItems().first()->text(ID_COLUMN);

    if (button == ui->songSecretCheckbox)
        songManager->updateSongFlags(id, SongManager::Flag::SECRET, checked);
    else if (button == ui->songDLCCheckbox)
        songManager->updateSongFlags(id, SongManager::Flag::DLC, checked);
    else if (button == ui->songDebugCheckbox)
        songManager->updateSongFlags(id, SongManager::Flag::DEBUG, checked);
    else if (button == ui->songRecordableCheckbox)
        songManager->updateSongFlags(id, SongManager::Flag::RECORDING, checked);;
}

void SongTabManager::multiSongPropButtonClicked(int id) {
    QList<QTreeWidgetItem *> songs = ui->songTree->selectedItems();
    SongManager::Flag key;
    bool set;

    switch (id) {
        case -2:
            // All secret
            key = SongManager::Flag::SECRET;
            set = true;
            break;
        case -3:
            // All DLC
            key = SongManager::Flag::DLC;
            set = true;
            break;
        case -4:
            // All debug
            key = SongManager::Flag::DEBUG;
            set = true;
            break;
        case -5:
            // All recordable
            key = SongManager::Flag::RECORDING;
            set = true;
            break;
        case -6:
            // None secret
            key = SongManager::Flag::SECRET;
            set = false;
            break;
        case -7:
            // None DLC
            key = SongManager::Flag::DLC;
            set = false;
            break;
        case -8:
            // None debug
            key = SongManager::Flag::DEBUG;
            set = false;
            break;
        case -9:
            // None recordable
            key = SongManager::Flag::RECORDING;
            set = false;
            break;
        default:
            qWarning("Invalid button ID");
            return;
    }

    for (auto &item : songs)
        songManager->updateSongFlags(item->text(ID_COLUMN), key, set);
}

void SongTabManager::songUpdated(const QString &id) {
    QList<QTreeWidgetItem *> items = ui->songTree->findItems(id, Qt::MatchExactly, ID_COLUMN);

    if (items.empty())
        return;

    QTreeWidgetItem *item = items.first();
    item->setText(MODIFIED_COLUMN, "*");
}

void SongTabManager::songTreeContextMenu(const QPoint &pos) {
    QList<QTreeWidgetItem *> songs = ui->songTree->selectedItems();

    if (songs.empty())
        return;

    QMenu *contextMenu(new QMenu);
    QMenu *setAllMenu = contextMenu->addMenu("Set selected...");
    QMenu *setNoneMenu = contextMenu->addMenu("Set selected not...");

    QAction *secretAction(new QAction("Secret"));
    QAction *dlcAction(new QAction("DLC"));
    QAction *debugAction(new QAction("Debug"));
    QAction *recordableAction(new QAction("Recordable"));
    QAction *noneSecretAction(new QAction("Secret"));
    QAction *noneDLCAction(new QAction("DLC"));
    QAction *noneDebugAction(new QAction("Debug"));
    QAction *noneRecordableAction(new QAction("Recordable"));


    auto actionTriggered = [=](QAction *action) {
        SongManager::Flag key;
        bool set = (action == secretAction || action == dlcAction || action == debugAction ||
                    action == recordableAction);

        if (action == secretAction || action == noneSecretAction)
            key = SongManager::Flag::SECRET;
        else if (action == dlcAction || action == noneDLCAction)
            key = SongManager::Flag::DLC;
        else if (action == debugAction || action == noneDebugAction)
            key = SongManager::Flag::DEBUG;
        else if (action == recordableAction || action == noneRecordableAction)
            key = SongManager::Flag::RECORDING;

        for (auto &item : songs)
            songManager->updateSongFlags(item->text(ID_COLUMN), key, set);
    };

    connect(contextMenu, &QMenu::triggered, this, actionTriggered);

    setAllMenu->addAction(secretAction);
    setAllMenu->addAction(dlcAction);
    setAllMenu->addAction(debugAction);
    setAllMenu->addAction(recordableAction);

    setNoneMenu->addAction(noneSecretAction);
    setNoneMenu->addAction(noneDLCAction);
    setNoneMenu->addAction(noneDebugAction);
    setNoneMenu->addAction(noneRecordableAction);

    contextMenu->exec(ui->songTree->mapToGlobal(pos));

    delete contextMenu;
}

void SongTabManager::addToBuildButtonClicked() {
    QList<QTreeWidgetItem *> songs = ui->songTree->selectedItems();
    QStringList ids;

    for (auto &song : songs)
        ids << song->text(ID_COLUMN);

    songManager->stageSongs(ids);
}

void SongTabManager::removeFromBuildButtonClicked() {
    QList<QTreeWidgetItem *> songs = ui->songTree->selectedItems();
    QStringList ids;

    for (auto &song : songs)
        ids << song->text(ID_COLUMN);

    songManager->unstageSongs(ids);
}
