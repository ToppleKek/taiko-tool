#ifndef TAIKO_TOOL_CONVERT_H
#define TAIKO_TOOL_CONVERT_H

#include <QByteArray>
#include <QFile>
#include <QDebug>
#include <QProcess>
#include <QSettings>
#include <stdexcept>

#include <Utils.h>

#define MAX_ID_LEN 5
#define SIZE_OFFSET_1 0x4c
#define SIZE_OFFSET_2 0x1450
#define STREAM_NAME_OFFSET_1 0xa5
#define STREAM_NAME_OFFSET_2 0x1439

namespace Convert {
    static void orbisSongToNX(QSettings *settings, const QString &orbisN3BPath, const QString &outPath, const QString &id) {
        QString vgmstreamPath = settings->value("vgmstream/path").toString();
        QString vgaudioPath = settings->value("vgaudio/path").toString();
        QFile templateFile(":/n3b/template.nus3bank");
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
        if (QProcess::execute(vgaudioPath, QStringList() << outPath + ".wav" << outPath + ".nop" << "--opusheader" << "Namco") != 0)
            throw std::runtime_error("VGAudio exited with non-zero exit code.");

        QFile wavFile(outPath + ".wav");
        QFile opusFile(outPath + ".nop");
        opusFile.open(QIODevice::ReadOnly);
        opusData = opusFile.readAll();

        templateFile.open(QIODevice::ReadOnly);

        templateData = templateFile.readAll();
        templateData.append(opusData);

        // Write OPUS data sizes
        Utils::write32(SIZE_OFFSET_1, opusData.length(), templateData);
        Utils::write32(SIZE_OFFSET_2, opusData.length(), templateData);

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

    static void wiiuFumenToNX(const QString &wiiuFumenPath, const QString &outPath) {
        QFile wiiuFumenFile(wiiuFumenPath);
        QFile outFile(outPath);

        wiiuFumenFile.open(QIODevice::ReadOnly);
        outFile.open(QIODevice::WriteOnly);
        QDataStream wiiuData(&wiiuFumenFile);
        QDataStream outData(&outFile);

        wiiuData.setByteOrder(QDataStream::BigEndian);
        outData.setByteOrder(QDataStream::LittleEndian);

        // Adapted from: https://pastebin.com/yx8v9MkG
        // Hantei notes
        quint32 data;
        for (int i = 0; i < 0x200; i += 4) {
            wiiuData >> data;
            outData << data;
        }

        quint32 numSections;
        quint32 bpm;
        quint32 startTime;
        quint8 gogo;
        quint8 sectionLine;
        quint32 bunki;
        quint16 numNotes;
        quint32 scroll;
        quint32 noteType;
        quint32 header;
        quint32 item;
        quint16 hit;
        quint16 scoreInc;
        quint32 length;

        quint16 unknown16;
        quint32 unknown32;

        wiiuData >> numSections;
        wiiuData >> unknown32;
        outData << numSections;
        outData << unknown32;

        for (int i = 0; i < numSections; i++) {
            wiiuData >> bpm;
            wiiuData >> startTime;
            wiiuData >> gogo;
            wiiuData >> sectionLine;
            wiiuData >> unknown16;
            outData << bpm;
            outData << startTime;
            outData << gogo;
            outData << sectionLine;
            outData << unknown16;

            // bunki
            for (int j = 0; j < 6; j++) {
                wiiuData >> bunki;
                outData << bunki;
            }

            wiiuData >> unknown32;
            outData << unknown32;

            // routes
            for (int j = 0; j < 3; j++) {
                wiiuData >> numNotes;
                wiiuData >> unknown16;
                wiiuData >> scroll;
                outData << numNotes;
                outData << unknown16;
                outData << scroll;

                // notes
                for (int k = 0; k < numNotes; k++) {
                    wiiuData >> noteType;
                    wiiuData >> header;
                    wiiuData >> item;
                    wiiuData >> unknown32;
                    wiiuData >> hit;
                    wiiuData >> scoreInc;
                    wiiuData >> length;
                    outData << noteType;
                    outData << header;
                    outData << item;
                    outData << unknown32;
                    outData << hit;
                    outData << scoreInc;
                    outData << length;

                    if (noteType == 6 || noteType == 9 || noteType == 98) {
                        wiiuData >> unknown32;
                        outData << unknown32;
                        wiiuData >> unknown32;
                        outData << unknown32;
                    }
                }
            }
        }

        outFile.close();
        wiiuFumenFile.close();
    }
}

#endif //TAIKO_TOOL_CONVERT_H
