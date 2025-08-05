#include <QLogBook.h>
#include <QLineEdit>
#include <QDebug>
#include <QString>

#include <stdlib.h>

#include "worker.h"

Worker::Worker() {
    resultfreq = "%1";
    freq = 0;
    submode = "";
    tcp = NULL;
}

Worker::~Worker() {
}

void Worker::quit() {
    stopme = true;
}

void Worker::process() {
    while ( !stopme ) {

        if ( tcp != NULL ) { // prepare reconnection to dead socket
            if ( tcp->state() == QAbstractSocket::UnconnectedState ) {
                // qDebug() << "close for reconnect because of state:" << tcp->state();
                tcp->close();
                tcp = NULL;
                QThread::msleep(3000);
            }
       }

        if ( tcp == NULL ) { // socket was dead or never connected
            if ( !host.isEmpty() && !port == 0 ) {
                tcp = new QTcpSocket();
                tcp->connectToHost(host, port);
                if ( !tcp->isOpen() ) {
                    qDebug() << "TCP connect failed! Port or Host wrong? YourPort:" << port << "YourHost:" << host;
                }
                worker->tcp->waitForConnected();
            }
        }

        //      qDebug() << "state:" << tcp->state();

        if ( tcp->state() == QAbstractSocket::UnconnectedState ) // socket has gone/rigctld closed, prepare reconnect
            continue;

        if ( stopme )
            break;

        freq = HamlibFreq();
        if ( freq > 0 ) {
            emit sigfreq(freq);
        }

        if ( stopme )
            break;

        submode = HamlibMode();
        if ( !submode.isNull() ) {
            if ( submode.length() > 0 )
                emit sigmode( submode );
        }

        if ( stopme )
            break;

        QThread::msleep(1000);
    }
    emit finished();
}
