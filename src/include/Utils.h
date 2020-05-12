#ifndef TAIKO_TOOL_UTILS_H
#define TAIKO_TOOL_UTILS_H

#include <QFile>
#include <QDir>
#include <QDebug>

namespace Utils {
    static void copyDir(const QDir &from, const QDir &to, const QString &before = "", const QString &after = "") {
        for (auto &filename : from.entryList(QDir::Files)) {
            QString newFilename = filename;
            newFilename.replace(before, after);
            QFile::copy(from.filePath(filename), to.path() + "/" + newFilename);
        }

        for (auto &dirname : from.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
            to.mkpath("./" + dirname);
            copyDir(from.filePath(dirname), to.path() + "/" + dirname);
        }
    }

    static int indexOfKeyValue(const QJsonArray &arr, const QString &key, const QString &value) {
        for (int i = 0; i < arr.size(); i++) {
            if (arr[i].toObject().contains(key) && arr[i].toObject()[key].toString() == value)
                return i;
        }

        return -1;
    }

    static int indexOfString(const QStringList &arr, const QString &string) {
        for (int i = 0; i < arr.size(); i++) {
            if (arr.at(i) == string)
                return i;
        }

        return -1;
    }

    static void write32LE(uint32_t offset, uint32_t value, QByteArray &byteArray) {
        uint8_t *p = (uint8_t *) &value;

        for (int i = 0; i < 4; i++, offset++)
            byteArray[offset] = p[i];
    }

    static void writeString(uint32_t offset, const char *str, QByteArray &byteArray) {
        int n = strlen(str);

        for (int i = 0; i < n; i++, offset++)
            byteArray[offset] = str[i];
    }
}


#endif //TAIKO_TOOL_UTILS_H
