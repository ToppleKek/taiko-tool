#include <QApplication>

#include <TaikoTool.h>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    TaikoTool window;
    window.show();

    return app.exec();
}
