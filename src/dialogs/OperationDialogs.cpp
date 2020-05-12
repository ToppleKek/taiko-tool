#include <dialogs/OperationDialogs.h>

SongDuplicateDialog::SongDuplicateDialog(QWidget *parent, const TaikoSong &loaded) : ui(new Ui::SongDuplicateDialog), intent(Intent::NO) {
    ui->setupUi(this);

    ui->infoLabel->setText("The song " + loaded.getTitleEN() + " (" + loaded.getId() + ") is already loaded from source: " +
                           loaded.getTaikoToolSource() + ". Would you like to replace this loaded song?");

    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, [=](QAbstractButton *button) {
        if (button == ui->buttonBox->button(QDialogButtonBox::NoToAll)) {
            intent = Intent::NO_TO_ALL;
            this->reject();
        } else if (button == ui->buttonBox->button(QDialogButtonBox::No)) {
            intent = Intent::NO;
            this->reject();
        } else if (button == ui->buttonBox->button(QDialogButtonBox::YesToAll)) {
            intent = YES_TO_ALL;
            this->accept();
        } else {
            intent = Intent::YES;
            this->accept();
        }
    });
}

SongDuplicateDialog::Intent SongDuplicateDialog::getIntent() const {
    return intent;
}
