#ifndef WORKER_H
#define WORKER_H

#include <QThread>
#include <QTcpSocket>

class Worker : public QObject {
    Q_OBJECT

public:
    Worker();
    ~Worker();
    QTcpSocket *tcp;

public slots:
    void process();
    void quit();

signals:
    void finished();
    void sigfreq(long f);
    void sigmode(QString m);

private:
    long freq = 0;
    QString resultfreq;
    bool stopme = false;
    QString submode;
};


#endif // WORKER_H
