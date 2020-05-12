#ifndef TAIKO_TOOL_OPERATIONDIALOGS_H
#define TAIKO_TOOL_OPERATIONDIALOGS_H

#include <QDialog>
#include <QPushButton>

#include <TaikoSong.h>
#include "ui_SongDuplicateDialog.h"

class SongDuplicateDialog : public QDialog {
public:
    enum Intent {
        YES_TO_ALL,
        YES,
        NO_TO_ALL,
        NO
    };

    SongDuplicateDialog(QWidget *parent, const TaikoSong &loaded);
    ~SongDuplicateDialog() = default;

    Intent getIntent() const;

private:
    Ui::SongDuplicateDialog *ui;
    Intent intent;

};

#endif //TAIKO_TOOL_OPERATIONDIALOGS_H
