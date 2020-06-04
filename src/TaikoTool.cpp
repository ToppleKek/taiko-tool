#include <TaikoTool.h>

TaikoTool::TaikoTool(QMainWindow *parent) : QMainWindow(parent), ui(new Ui::TaikoTool),
                                            settings(new QSettings("ToppleKek", "taiko-tool")) {
    Convert::wiiuFumenToNX("/home/topkek/testTaikoFolder/wiiu_stuff/natsu_m.bin", "/home/topkek/testTaikoFolder/wiiu_stuff/output.bin");
    songManager = new SongManager(settings);
    ui->setupUi(this);

    songTabManager = new SongTabManager(ui, songManager, settings);
    buildTabManager = new BuildTabManager(ui, songManager);

    ui->songTree->setColumnWidth(0, 100);
    ui->songTree->setColumnWidth(1, 200);
    ui->songTree->setColumnWidth(2, 200);
    ui->buildButtonWidget->setVisible(false);

    connect(ui->actionBase_game, &QAction::triggered, this, &TaikoTool::baseGameActionTriggered);
    connect(ui->actionPS4Game, &QAction::triggered, this, &TaikoTool::ps4GameActionTriggered);
    connect(ui->actionSettings, &QAction::triggered, this, &TaikoTool::settingsActionTriggered);

    connect(songManager, &SongManager::songsStaged, this, [=](int numberStaged) {
        statusBar()->showMessage(QString::number(numberStaged) + " songs staged for building", 5000);
        buildTabManager->updateList();
    });

    connect(songManager, &SongManager::songsUnstaged, this, [=](int numberUnstaged) {
        statusBar()->showMessage(QString::number(numberUnstaged) + " songs removed from build stage", 5000);
        buildTabManager->updateList();
    });

    connect(songManager, &SongManager::buildCompleted, this, [=]() {
        statusBar()->showMessage("Build completed", 5000);
    });

    QString cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QDir cacheDir(cachePath);

    cacheDir.mkpath("./sound");
}

void TaikoTool::baseGameActionTriggered() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select base game folder...", QDir::currentPath(),
                                                    QFileDialog::ShowDirsOnly);

    QDir baseGameDir = QDir(dir + "/Data/NX");

    QFile musicInfo(baseGameDir.path() + "/datatable/musicinfo.bin");
    QFile wordList(baseGameDir.path() + "/datatable/wordlist.bin");
    QDir songs(baseGameDir.path() + "/sound");
    QDir maps(baseGameDir.path() + "/");

    if (!baseGameDir.exists() || !musicInfo.exists() || !wordList.exists() || !songs.exists()) {
        QMessageBox::critical(this, "Invalid base game directory",
                              "This directory does not contain the required game files. Ensure that this directory contains the files in: Data/NX");
        return;
    }

    try {
        songManager->processGameDir(baseGameDir, "Base game",true);
        songTabManager->updateTree(songManager->getLoadedSongs());
    } catch (boost::exception &e) {
        QMessageBox::critical(this, "Unreadable music configuration",
                             "Taiko Tool was unable to extract music configuration from this game version (all versions newer than 1.0.0 cannot be read).");
    }

    statusBar()->showMessage("Loaded base game songs", 5000);
}

void TaikoTool::ps4GameActionTriggered() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select PS4 game folder...", QDir::currentPath(),
                                                    QFileDialog::ShowDirsOnly);

    QDir baseGameDir = QDir(dir + "/Data/ORBIS");

    QFile musicInfo(baseGameDir.path() + "/datatable/musicinfo.bin");
    QFile wordList(baseGameDir.path() + "/datatable/wordlist.bin");
    QDir songs(baseGameDir.path() + "/sound");
    QDir maps(baseGameDir.path() + "/");

    if (!baseGameDir.exists() || !musicInfo.exists() || !wordList.exists() || !songs.exists()) {
        QMessageBox::critical(this, "Invalid PS4 game directory",
                              "This directory does not contain the required game files. Ensure that this directory contains the files in: Data/ORBIS");
        return;
    }

    try {
        songManager->processGameDir(baseGameDir, "PS4");
        songTabManager->updateTree(songManager->getLoadedSongs());
    } catch (boost::exception &e) {
        QMessageBox::critical(this, "Unreadable music configuration",
                              "Taiko Tool was unable to extract music configuration from this game.");
    }

    statusBar()->showMessage("Loaded PS4 songs", 5000);
}

void TaikoTool::settingsActionTriggered() {
    SettingsDialog settingsDialog(nullptr, settings);
    settingsDialog.exec();
}
