#include <dialogs/SettingsDialog.h>

SettingsDialog::SettingsDialog(QDialog *parent, QSettings *settings) : QDialog(parent), ui(new Ui::SettingsDialog) {
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    ui->vsLineEdit->setText(settings->value("vgmstream/path").toString());
    ui->vaLineEdit->setText(settings->value("vgaudio/path").toString());

    connect(ui->vsBrowseButton, &QPushButton::clicked, this, [=]() {
        QString newPath = QFileDialog::getOpenFileName(this, "Select vgmstream-cli binary...");

        if (!newPath.isEmpty())
            ui->vsLineEdit->setText(newPath);
    });

    connect(ui->vaBrowseButton, &QPushButton::clicked, this, [=]() {
        QString newPath = QFileDialog::getOpenFileName(this, "Select VGAudioCli binary...");

        if (!newPath.isEmpty())
            ui->vaLineEdit->setText(newPath);
    });

    connect(ui->settingsButtonBox, &QDialogButtonBox::accepted, this, [=]() {
        settings->setValue("vgmstream/path", ui->vsLineEdit->text());
        settings->setValue("vgaudio/path", ui->vaLineEdit->text());

        this->accept();
    });

    connect(ui->settingsButtonBox, &QDialogButtonBox::rejected, this, [=]() {
        this->reject();
    });
}
