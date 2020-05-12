#include <BuildTabManager.h>

BuildTabManager::BuildTabManager(Ui::TaikoTool *ui, SongManager *songManager) : ui(ui), songManager(songManager) {
    connect(songManager, &SongManager::songsStaged, this, &BuildTabManager::songsStaged);
    connect(ui->buildButton, &QPushButton::clicked, this, &BuildTabManager::buildButtonClicked);
}

void BuildTabManager::updateList() {
    ui->buildList->clear();
    QList<TaikoSong> songs = songManager->collectStagedSongs();

    if (songs.isEmpty()) {
        ui->buildReadyLabel->setText("Add songs to the build in the \"All Songs\" tab");
        ui->buildButton->setEnabled(false);
        return;
    } else {
        ui->buildReadyLabel->setText("Ready to build");
        ui->buildButton->setEnabled(true);
    }

    for (const auto &song : songs)
        ui->buildList->addItem(new QListWidgetItem(song.getTitleEN()));
}

void BuildTabManager::songsStaged(int numberStaged) {

}

void BuildTabManager::buildButtonClicked() {
    SongManager::BuildOptions buildOptions = SongManager::BuildOptions();
    buildOptions.outDir.setPath(QFileDialog::getExistingDirectory(nullptr, "Select output folder...", QDir::currentPath(),
                                                                  QFileDialog::ShowDirsOnly));
    buildOptions.showVersionMainMenu = ui->showVersionMSCheckbox->isChecked();
    buildOptions.showVersionSongSelect = ui->showVersionSSCheckbox->isChecked();
    buildOptions.showSourceSubText = ui->showSourceCheckbox->isChecked();

    songManager->build(buildOptions);
}
