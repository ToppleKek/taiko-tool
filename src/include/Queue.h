#ifndef TAIKO_TOOL_QUEUE_H
#define TAIKO_TOOL_QUEUE_H

#include <QMutex>
#include <QMutexLocker>
#include <QQueue>
#include <QList>
#include <QThread>
#include <QSettings>

#include <TaikoSong.h>
#include <Nus3bank.h>

template <class T>
class Queue {
public:
    void enqueue(T item) {
        QMutexLocker lock(&mutex);

        list.prepend(item);
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
    void doWork(Queue<TaikoSong> *queue, QSettings *settings, const QDir *outDir) {
        finished = false;

        for (;;) {
            TaikoSong song;

            if (!queue->dequeue(song))
                break;

            QString outPath = outDir->path() + "/" + song.getSongFileName() + ".nus3bank";
            Nus3bank::orbisToNX(settings, song.getSongFilePath(), outPath, song.getId());

            emit workerProgress();
        }

        finished = true;
        emit workerFinished();
    }

signals:
    void workerProgress();
    void workerFinished();

private:
    bool finished;
};

class QueueProcessor : public QObject {
    Q_OBJECT

public:
    QueueProcessor(Queue<TaikoSong> *queue, int numThreads) : queue(queue) {
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

    void start(QSettings *settings, const QDir *outDir) {
        emit run(queue, settings, outDir);
    }

    void stop();

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

private:
    Queue<TaikoSong> *queue;
    QList<QueueWorker *> workers;
    QList<QThread *> threads;

signals:
    void run(Queue<TaikoSong> *queue, QSettings *settings, const QDir *outDir);
    void finished();
    void halt();
    void progress();
};

#endif //TAIKO_TOOL_QUEUE_H
