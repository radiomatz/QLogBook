#include "QLogBook.h"
#include <QTcpSocket>
#include <memory.h>
#include <string.h>
#include <unistd.h>

bool debughamlib = false;

long HamlibFreq() {
    if ( host.isEmpty() || port == 0 )
        return(0);

    long freq;
    char msg[128];
    memset(msg,0,128);

    if ( worker->tcp->isOpen() ) {
        if(worker->tcp->isWritable() ) {
            if ( debughamlib ) qDebug() << "tcp is writable";
            worker->tcp->write("f\n", 2);
        } else {
            if ( debughamlib ) qDebug() << "tcp not writable";
            return(0);
        }
        worker->tcp->waitForReadyRead();
        if ( worker->tcp->isReadable() ) {
            worker->tcp->readLine(msg, 128);
            freq = QString(msg).toLong();
            if ( debughamlib ) qDebug() << "worker->tcp->read" << msg;
        } else {
            if ( debughamlib ) qDebug() << "tcp not readable";
            return(0);
        }
    } else {
        qDebug() << "tcp not connected";
        return(0);
    }
    if ( debughamlib ) qDebug() << "return " << freq;
    return ( freq );
}

QString HamlibMode() {
    if ( host.isEmpty() || port == 0 )
        return(0);

    QString mode;
    char msg[128];
    memset(msg,0,128);

    if ( worker->tcp->isOpen() ) {
        if(worker->tcp->isWritable() ) {
            if ( debughamlib ) qDebug() << "tcp(mode) is writable";
            worker->tcp->write("m\n", 2);
        } else {
            if ( debughamlib ) qDebug() << "tcp(mode) not writable";
            return(0);
        }
        worker->tcp->waitForReadyRead();
        if ( worker->tcp->isReadable() ) {
            worker->tcp->readLine(msg, 128);
            mode = QString(msg);
            if ( debughamlib ) qDebug() << "tcp(mode).read" << msg;
        } else {
            if ( debughamlib ) qDebug() << "tcp(mode) not readable";
            return(0);
        }
        if ( worker->tcp->isReadable() )
            worker->tcp->readLine(msg, 128);
    } else {
        qDebug() << "tcp(mode) not connected";
        return(0);
    }
    if ( debughamlib ) qDebug() << "return(mode) " << mode;
    return ( mode );
}
