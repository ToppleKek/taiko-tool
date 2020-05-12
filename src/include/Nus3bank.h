#ifndef TAIKO_TOOL_NUS3BANK_H
#define TAIKO_TOOL_NUS3BANK_H

#include <QByteArray>
#include <QFile>
#include <QDebug>
#include <QProcess>
#include <QSettings>
#include <stdexcept>

#include <Utils.h>

#define TEMPLATE_PATH "./template.nus3bank"
#define MAX_ID_LEN 5
#define SIZE_OFFSET_1 0x4c
#define SIZE_OFFSET_2 0x1450
#define STREAM_NAME_OFFSET_1 0xa5
#define STREAM_NAME_OFFSET_2 0x1439

namespace Nus3bank {
    static void orbisToNX(QSettings *settings, const QString &orbisN3BPath, const QString &outPath, const QString &id) {
        QString vgmstreamPath = settings->value("vgmstream/path").toString();
        QString vgaudioPath = settings->value("vgaudio/path").toString();
        QFile templateFile(TEMPLATE_PATH);
        QFile orbisFile(orbisN3BPath);
        QFile outFile(outPath);
        QByteArray templateData;
        QByteArray opusData;

        if (id.length() > MAX_ID_LEN)
            throw std::invalid_argument("Provided ID is too long to fit in nus3bank.");

        if (!orbisFile.exists())
            throw std::invalid_argument("Provided PS4 ns3bank file does not exist.");

        if (!QFile(vgmstreamPath).exists() || !QFile(vgaudioPath).exists())
            throw std::invalid_argument("Provided paths to vgmstream and vgaudio do not exist.");

        if (outFile.exists())
            outFile.remove();

        // Convert the input PS4 song to wav with vgmstream
        if (QProcess::execute(vgmstreamPath, QStringList() << "-o" << outPath + ".wav" << orbisN3BPath) != 0)
            throw std::runtime_error("vgmstream exited with non-zero exit code.");

        // Convert the outputted wav file to NX OPUS data with vgaudio
        if (QProcess::execute(vgaudioPath,
                              QStringList() << outPath + ".wav" << outPath + ".nop" << "--opusheader" << "Namco") != 0)
            throw std::runtime_error("VGAudio exited with non-zero exit code.");

        QFile wavFile(outPath + ".wav");
        QFile opusFile(outPath + ".nop");
        opusFile.open(QIODevice::ReadOnly);
        opusData = opusFile.readAll();

        templateFile.open(QIODevice::ReadOnly);

        templateData = templateFile.readAll();
        templateData.append(opusData);

        // Write OPUS data sizes
        Utils::write32LE(SIZE_OFFSET_1, opusData.length(), templateData);
        Utils::write32LE(SIZE_OFFSET_2, opusData.length(), templateData);

        // Write stream names
        Utils::writeString(STREAM_NAME_OFFSET_1, "SONG_", templateData);
        Utils::writeString(STREAM_NAME_OFFSET_1 + 5, id.toUpper().toStdString().c_str(), templateData);
        Utils::writeString(STREAM_NAME_OFFSET_2, "SONG_", templateData);
        Utils::writeString(STREAM_NAME_OFFSET_2 + 5, id.toUpper().toStdString().c_str(), templateData);

        outFile.open(QIODevice::WriteOnly);
        outFile.write(templateData);
        outFile.close();

        wavFile.remove();
        opusFile.remove();
    }
}

#endif //TAIKO_TOOL_NUS3BANK_H
