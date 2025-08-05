#include "QLogBook.h"
#include <db.h>
#include <string.h>
#include <stdlib.h>
#include <QMessageBox>
#include <QSqlQuery>
#include <QProcess>
#include <QTableView>
#include <QDateTime>
#include <QDate>
#include <QTime>
#include <QString>
#include <QDir>
#include <QComboBox>
#include <QLineEdit>
#include "callfield.h"
#include <QDebug>
#include <QListView>
#include <QItemDelegate>
#include <noteditabledelegate.h>
#include <myitemdelegate.h>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QStringList>

#include "bands.cpp"

bool doquery(QString qsz);

QString find_band ( float freq ) {
    int i;
    for ( i = 0; band[i].mtr; i++ ) {
        if ( band[i].lower <= freq && band[i].upper >= freq ) {
            break;
        }
    }
    return QString(band[i].mtr);
}


bool open_db() {
    bool reopen = false;

    if ( db.isOpen() ) {
        db.close();
        reopen = true;
    }
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbpath + QDir::separator() + "sqlite3.db");
    bool dbok = db.open();
    if ( !dbok ) {
        QMessageBox msgbox( QMessageBox::Critical ,
                       "DB open Error",
                       QString("Can not open Database: %1").arg(db.lastError().text() ) );
        msgbox.exec();
        return false;
    }
    if ( reopen ) {
        mtv->select();
        mtvd->clear();
    }
    if ( conf.value("dbbackup").toInt() == 1 ) {
        QDateTime UTC(QDateTime::currentDateTime().toUTC());
        QString dt = UTC.toString("yyyyMMdd");
        QString fn(dbpath + QDir::separator() + dt + "_sqlite3.db");
        QString origfn(dbpath + QDir::separator() + "sqlite3.db");
        QFile bdb(fn);
        if ( !bdb.exists() ) {
            qDebug() << "backup db:" << origfn << "to:" << fn;
            QFile::copy(origfn, fn);
        }
    }
    return true;
}


bool update_fields() {
    uip->mycall->setText(mycall);
    QStringList modes = {"AM", "ARDOP", "ATV", "CHIP", "CLO", "CONTESTI", "CW", "DIGITALVOICE", "DOMINO", "DYNAMIC", "FAX", "FM", "FSK441", "FT8", "HELL", "ISCAT", "JT4", "JT6M", "JT9", "JT44", "JT65", "MFSK", "MSK144", "MT63", "OLIVIA", "OPERA", "PAC", "PAX", "PKT", "PSK", "PSK2K", "Q15", "QRA64", "ROS", "RTTY", "RTTYM", "SSB", "SSTV", "T10", "THOR", "THRB", "TOR", "V4", "VOI", "WINMOR", "WSPR"};
    QComboBox *cmode = uip->cmode;
    cmode->addItems(modes);
    cmode->setCurrentIndex(cmode->findText("SSB"));
    return true;
}

bool generate_qsolist(MainWindow *parent) {
    QString ltabelle = "vqso"; // vqso is a view of table qso

    mtv = new QSqlTableModel(NULL, db);
    mtv->setTable(ltabelle);
    mtv->setEditStrategy(QSqlTableModel::OnFieldChange);
    mtv->setHeaderData(0, Qt::Horizontal, "QsoNr");
    mtv->setHeaderData(1, Qt::Horizontal, "Callsign");
    mtv->setHeaderData(2, Qt::Horizontal, "Date");
    mtv->setHeaderData(3, Qt::Horizontal, "Time");
    mtv->setHeaderData(4, Qt::Horizontal, "Band");
    mtv->setHeaderData(5, Qt::Horizontal, "Mode");
    mtv->setHeaderData(6, Qt::Horizontal, "Pwr");
    mtv->select();

    QTableView* tv = uip->qsoView;
    tv->setModel(mtv);
    // Make all the columns except the third(value) read only
    for(int i = 0; i < mtv->columnCount(); i++) {
        tv->setItemDelegateForColumn(i, new NotEditableDelegate(tv));
    }
    // tv->hideColumn(0); // don't show the ID
    tv->resizeColumnsToContents();
    tv->setMinimumWidth(440);
    tv->show();
    generate_qsodetaillist(parent, mtv->index(0,0).data().toInt());

    return true;
};


bool generate_qsodetaillist(MainWindow *parent, int nr) {
    QString ltabelle = "qsod";

    mtvd = new QSqlTableModel(NULL, db);
    mtvd->setTable(ltabelle);
    mtvd->setEditStrategy(QSqlTableModel::OnManualSubmit);
    mtvd->setFilter(QString("nr=%1").arg(nr));
    mtvd->select();
    mtvd->setHeaderData(0, Qt::Horizontal, "QsoNr");
    mtvd->setHeaderData(1, Qt::Horizontal, "Field");
    mtvd->setHeaderData(2, Qt::Horizontal, "Value");


    QTableView* tvd = uip->qsodView;
    tvd->setModel(mtvd);
    // Make all the columns except the third(value) read only
    for(int i = 0; i < mtvd->columnCount(); i++) {
        if(i != 2)
            tvd->setItemDelegateForColumn(i, new NotEditableDelegate(tvd));
//        else
//            tvd->setItemDelegateForColumn(i, new MyItemDelegate(tvd));
    }
    // tvd->hideColumn(0); // don't show the ID
    tvd->resizeColumnsToContents();


    tvd->show();

    return true;
};


bool doquery(QString qsz) {
    QSqlQuery qry(db);
    qry.prepare(qsz);
    if ( debugsql )
        qDebug() << qsz.toLocal8Bit();
    qry.exec();
    QSqlError sqlerror = db.lastError();
    if ( sqlerror.type() != QSqlError::NoError ) {
        qDebug() << "type != noerror:" << sqlerror.text();
    }
    if ( sqlerror.isValid() ) {
        qDebug() << "isvalid:" << sqlerror.text();
    }
    return true;
}


int create_table_if_not_exist() {
    bool olddebugsql = debugsql;
    debugsql = false;
    doquery("create table if not exists tnr(nr integer);");
    doquery("delete from tnr;");
    doquery("insert into tnr values(0);");

    doquery("create table if not exists qso(nr integer primary key,call collate nocase,date int,time int,band collate nocase,mode collate nocase,pwr int);");
    doquery("create unique index if not exists iqsocall on qso(call collate nocase,date,time);");
    doquery("create unique index if not exists iqsodate on qso(date,time,call collate nocase);");


    doquery("create trigger if not exists trcall after update of value on qsod when old.field='call' begin update qso set call=new.value where qso.nr=old.nr; end;");
    doquery("create trigger if not exists trdate after update of value on qsod when old.field='qso_date' begin update qso set date=new.value where qso.nr=old.nr; end;");
    doquery("create trigger if not exists trtime after update of value on qsod when old.field='time_on' begin update qso set time=new.value where qso.nr=old.nr; end;");
    doquery("create trigger if not exists trband after update of value on qsod when old.field='band' begin update qso set band=new.value where qso.nr=old.nr; end;");
    doquery("create trigger if not exists trmode after update of value on qsod when old.field='mode' begin update qso set mode=new.value where qso.nr=old.nr; end;");
    doquery("create trigger if not exists trpwr after update of value on qsod when old.field='TX_PWR' begin update qso set pwr=new.value where qso.nr=old.nr; end;");
    doquery("create trigger if not exists itrpwr after insert on qsod when new.field='TX_PWR' begin update qso set pwr=new.value where qso.nr=new.nr; end;");

    doquery( "create table if not exists qsod(nr,field collate nocase,value collate nocase);");
    doquery( "create index if not exists iqsod on qsod(nr);");
    doquery( "create unique index if not exists iqsodf on qsod(nr,field collate nocase);");

    doquery("create view if not exists vqso (nr,call,date,time,band,mode,pwr) as select nr,call,concat(substr(date,1,4),'-',substr(date,5,2),'-',substr(date,7,2)) as date,concat(substr('000000',6-length(time),6-length(time)),substr(time,1,length(time)-4),':',substr(time,length(time)-3,2),':',substr(time,length(time)-1)) as time,band,mode,pwr from qso order by date desc,time desc;");

    debugsql = olddebugsql;
    return true;
}


bool show_qrz_window(QString call) {
    QStringList farg;

    if ( call.isEmpty() || call.length() < 4 ) {
        QTableView *tv =  pw->findChild<QTableView*>("qsoView");
        call = mtv->index(tv->currentIndex().row(),1).data().toString();
    }
    if ( call.isEmpty() ) {
        // QMessageBox::warning(0, "Empty", "Empty Call" );
        return false;
    }

    farg = browserargs.split(" ");
    if ( farg[0].length() == 0 )
        farg = { "https://www.qrz.com/db/" + call };
    else
        farg << "https://www.qrz.com/db/" + call;
    // qDebug() << "Browserargs:" << farg;
    if ( process->isOpen() )
        process->close();
    process->start(browser, farg);

    return true;
}


bool make_new_qso(QString callsign) {

    if ( callsign.isEmpty() )
        return false;

    QString qry("");

    QDateTime local(QDateTime::currentDateTime());
    QDateTime UTC(local.toUTC());

    QString dt = UTC.toString("yyyyMMdd"); // .toString("yyyy-MM-dd hh:mm:ss");
    QString tm = UTC.toString("hhmmss");

    QString mode = uip->cmode->currentText();
    QString rstin = uip->rstin->text();
    QString rstout = uip->rstout->text();
    QString exchin = uip->exchin->text();
    QString exchout = uip->exchout->text();
    QString contestid = uip->contestid->text();

    QString sfreq = uip->freq->text().replace(".","");
    float freq = sfreq.toDouble();
    freq /= 1000000.0F;
    QString band = find_band(freq);

    doquery("begin transaction;");

    qry = "insert into qso(call,date,time,band,mode) values('%1','%2','%3','%4','%5');";
    doquery(qry.arg(callsign, dt, tm, band, mode));
    doquery("update tnr set nr = last_insert_rowid();");

    qry  = "insert into qsod values((select nr from tnr limit 1),'%1','%2');";
    doquery(qry.arg("call", callsign));
    doquery(qry.arg("qso_date", dt));
    doquery(qry.arg("time_on", tm));
    doquery(qry.arg("band", band));
    doquery(qry.arg("freq").arg(freq));
    doquery(qry.arg("mode", mode));
    doquery(qry.arg("station_callsign", mycall));
    if ( mode.compare("SSB") == 0 && !pw->submode.isEmpty() ) {
        doquery(qry.arg("submode", pw->submode));
    }

    // Contest Fields
    if ( !contestid.isEmpty() )
        doquery(qry.arg("contest_id", contestid));
    if ( !rstin.isEmpty() )
        doquery(qry.arg("rst_rcvd", rstin));
    if ( !rstout.isEmpty() )
    doquery(qry.arg("rst_sent", rstout));
    if ( !exchin.isEmpty() )
        doquery(qry.arg("srx_string", exchin));
    if ( !exchout.isEmpty() )
        doquery(qry.arg("stx_string", exchout));

    // additional Fields from settings (static)
    QStringList additionalvalues = conf.value("additionalvalues").toStringList();
    qry  = "insert into qsod values((select nr from tnr limit 1),'%1','%2');";
    for ( int i = 0; i < additionalvalues.length(); i++ ) {
        if ( !additionalvalues[i].isEmpty() ) {
            doquery(qry.arg(adif_fields[i], additionalvalues[i]));
        }
    }

    doquery("commit transaction;");

    generate_qsodetaillist(pw, mtv->index(
        pw->findChild<QTableView*>("qsoView")->currentIndex().row(),0)
        .data().toInt());


    return true;
}

bool testnr(QString test, int *nr) {
bool bok;
    *nr = test.toInt(&bok);
    return(bok);
}


bool increment_counter() {
    QString exchout = uip->exchout->text();
    QStringList so;
    int nr;
    if ( exchout.length() >= 3 ) {
        so = exchout.split(" ");
        for(int i = 0; i < so.length(); i++ ) {
            if ( so[i].length() == 3 && testnr(so[i], &nr) ) {
                nr++;
                QString out;
                QTextStream ts(&out);
                ts.setFieldWidth(3);
                ts.setPadChar('0');
                ts << nr;
                so[i] = out;
                break;
            }
        }
        exchout = "";
        QString blank = "";
        for(int i = 0; i < so.length(); i++ ) {
            exchout = exchout + blank + so[i];
            blank = " ";
        }
    }
    uip->exchout->setText(exchout);
}


bool delete_qso(QModelIndexList qmi) {
    if ( qmi.count() == 0 )
        return false;

    bool bnosecuredelete = conf.value("dontaskfordelete", false).toBool();

    for (QList<QModelIndex>::iterator i = qmi.begin(); i != qmi.end(); i++) {
  //      qDebug() << *i;
        int row = i->row();
        int nr = mtv->index(row, 0).data().toInt();
        if ( nr == 0 )
           continue;
        if ( !bnosecuredelete ) {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(pw,
                        "Delete QSO",
                        "Delete QSO Nr: " + QString("%1").arg(nr),
                        QMessageBox::Yes|QMessageBox::No);
            if (reply != QMessageBox::Yes) {
                return false;
            }
        }
        QString qry = "delete from %1 where nr=%2;";
        doquery(qry.arg("qso").arg(nr));
        doquery(qry.arg("qsod").arg(nr));
//        qDebug() << qry.arg("qso").arg(nr) << qry.arg("qsod").arg(nr);
    }
    return true;
}


bool adif_header(QFile *f) {
    QDateTime local(QDateTime::currentDateTime());
    QDateTime UTC(local.toUTC());
    QString crdt = UTC.toString("yyyyMMdd hhmmss");

    QString header = QString("ADIF Export") + "\n" +
            "<adif_ver:5>3.1.5" + "\n" +
            "<created_timestamp:" + QString("%1").arg(crdt.length()) + ">" + crdt  + "\n" +
            "<programid:14>" + MY_PROG + " " + MY_ORG + "\n" +
            "<programversion:5>" + MY_VERSION + "\n" +
            "<eoh>" + "\n";
//    qDebug() << header;
    f->write(header.toLocal8Bit(),header.length());
}

bool adif_footer(QFile *f) {
    QString foot = "\n\n";
    // qDebug() << foot;
    f->write(foot.toLocal8Bit(),foot.length());
}

bool adif_record(QFile *f, int qsonr) {
    QString adiftags;
    // brain damaged import of arrl tqsl wants call first ...
    QString sqry("select * from qsod where nr=%1 and field='call' COLLATE NOCASE;");
    QSqlQuery qry(sqry.arg(qsonr));
//    qDebug() << sqry.arg(qsonr);
    while(qry.next()) {
//        qDebug() << sqry.arg(qsonr);
        QString val = qry.value(2).toString();
        QString adiftag = "<" + qry.value(1).toString() + ":" + QString("%1").arg(val.length()) + ">" + val;
        adiftags.append(adiftag);

        QString sqry2 = "select * from qsod where nr=%1 and not field='call' COLLATE NOCASE;";
        QSqlQuery qry2(sqry2.arg(qsonr));
//        qDebug() << sqry2.arg(qsonr);
        while(qry2.next()) {
            QString val = qry2.value(2).toString();
            QString adiftag = "<" + qry2.value(1).toString() + ":" + QString("%1").arg(val.length()) + ">" + val;
            adiftags.append(adiftag);
        }
    }
    adiftags.append("<eor>\n");
//    qDebug() << "qsonr:" << qsonr << adiftags;
    f->write(adiftags.toLocal8Bit(), adiftags.length());
}

bool export_adif(QFile *fi) {
    if ( !fi->open(QIODevice::ReadWrite | QIODevice::Text) ) {
        QMessageBox msgBox(QMessageBox::Critical, "File open error", "Error while opening output file:" + fi->fileName(), QMessageBox::Abort);
        msgBox.exec();
        return false;
    }
    QItemSelectionModel *select = uip->qsoView->selectionModel();
    if ( !select->hasSelection() ) {
        QMessageBox msgBox(QMessageBox::Critical, "No Range", "Nothing to Export selected", QMessageBox::Abort);
        msgBox.exec();
        return false;
    }
    adif_header(fi);
    QModelIndexList range = select->selectedRows(); // return selected row(s)
    for(int i = 0; i < range.length(); i++) {
//        qDebug() << range.at(i).row() << " : " << range.at(i).data().toInt();
        adif_record(fi, range.at(i).data().toInt() );
    }
    adif_footer(fi);
    fi->close();
    return true;
}

