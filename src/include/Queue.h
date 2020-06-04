#ifndef TAIKO_TOOL_QUEUE_H
#define TAIKO_TOOL_QUEUE_H

#include <QMutex>
#include <QMutexLocker>
#include <QList>
#include <QThread>
#include <QSettings>

#include <TaikoSong.h>
#include <Convert.h>

template <class T>
class Queue {
public:
    void enqueue(T item) {
        QMutexLocker lock(&mutex);

        list.append(item);
    }

    bool dequeue(T &value) {
        QMutexLocker lock(&mutex);

        if (list.isEmpty())
            return false;

        value = list.takeAt(0);

        return true;
    }

    int size() {
        QMutexLocker lock(&mutex);

        return list.size();
    }

private:
    QList<T> list;
    QMutex mutex;
};

class QueueWorker : public QObject {
    Q_OBJECT

public:
    bool isFinished() {
        return finished;
    }

public slots:
    void doWork(Queue<TaikoSong> *queue, QSettings *settings, const QDir *outDir, bool *canceled) {
        TaikoSong song;

        while(queue->dequeue(song) && !*canceled) {
            QString outPath = outDir->path() + "/" + song.getSongFileName() + ".nus3bank";
            Convert::orbisSongToNX(settings, song.getSongFilePath(), outPath, song.getId());

            emit workerProgress();
        }

        finished = true;
        emit workerFinished();
    }

signals:
    void workerProgress();
    void workerFinished();

private:
    bool finished = false;
};

class QueueProcessor : public QObject {
    Q_OBJECT

public:
    QueueProcessor(Queue<TaikoSong> *queue, QSettings *settings) : queue(queue), settings(settings), canceled(false) {
        int numThreads = 4;

        if (settings->contains("build/numThreads"))
            numThreads = settings->value("build/numThreads").toInt();

        for (int i = 0; i < numThreads; i++) {
            threads << new QThread;
            workers << new QueueWorker;
            workers[i]->moveToThread(threads[i]);

            connect(threads[i], &QThread::finished, workers[i], &QObject::deleteLater);
            connect(workers[i], &QueueWorker::workerProgress, this, &QueueProcessor::workerProgress);
            connect(workers[i], &QueueWorker::workerFinished, this, &QueueProcessor::workerFinished);
            connect(this, &QueueProcessor::run, workers[i], &QueueWorker::doWork);

            threads[i]->start();
        }
    }

    ~QueueProcessor() override {
        for (auto &thread : threads) {
            thread->quit();
            thread->wait();
        }
    }

    void start(const QDir *outDir) {
        emit run(queue, settings, outDir, &canceled);
    }

    void terminate() {
        canceled = true;
    }

public slots:
    void workerProgress() {
        emit progress();
    }

    void workerFinished() {
        for (auto worker : workers)
            if (!worker->isFinished())
                return;

        emit finished();
    }

signals:
    void run(Queue<TaikoSong> *queue, QSettings *settings, const QDir *outDir, bool *canceled);
    void finished();
    void progress();

private:
    Queue<TaikoSong> *queue;
    QList<QueueWorker *> workers;
    QList<QThread *> threads;
    QSettings *settings;
    bool canceled;
};

#endif //TAIKO_TOOL_QUEUE_H
