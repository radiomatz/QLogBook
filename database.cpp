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
#include <qobject.h>

#include "cabrilloheader.h"
#include "ui_cabrilloheader.h"
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
    tvd->hideColumn(0); // don't show the ID
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
    doquery(qry.arg("operator", mycall));
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

// ADIF export routines

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


// CABRILLO export routines

bool cabrillo_header(QFile *f, cabrilloheader *ch) {

    QString header = QString("START-OF-LOG:3.0\n\
CONTEST: " + ch->uip->contest->currentText() + "\n\
CALLSIGN: " + ch->uip->callsign->text().toUpper() + "\n\
GRID-LOCATOR: " + ch->uip->locator->text() + "\n\
LOCATION: " + ch->uip->location->text() + "\n\
CATEGORY-OPERATOR: " + ch->uip->callsign->text() + "\n\
CATEGORY-BAND: " + ch->uip->bands->currentText() + "\n\
CATEGORY-MODE: " + ch->uip->mode->currentText() + "\n\
CATEGORY-POWER: " + ch->uip->power->currentText() + "\n\
CATEGORY-STATION: " + ch->uip->station->currentText() + "\n\
CATEGORY-TIME: " + ch->uip->time->currentText() + "\n\
CATEGORY-TRANSMITTER: " + ch->uip->trx->currentText() + "\n\
CATEGORY-OVERLAY: " + ch->uip->overlay->currentText() + "\n\
CATEGORY-ASSISTED: " + ch->uip->assisted->currentText() + "\n\
CERTIFICATE: " + ch->uip->certificate->currentText() + "\n\
CLAIMED-SCORE: " + ch->uip->certificate->currentText() + "\n\
CLUB: " + ch->uip->club->text() + "\n\
CREATED-BY: " + QString(MY_PROG) + " " + QString(MY_ORG) + " " + QString(MY_VERSION) + "\n\
OPERATORS: " + ch->uip->operators->text() + "\n\
NAME: " + ch->uip->name->text() + "\n\
EMAIL: " + ch->uip->email->text() + "\n\
ADDRESS: " + ch->uip->address->text() + "\n\
ADDRESS-POSTALCODE: " + ch->uip->postalcode->text() + "\n\
ADDRESS-CITY: " + ch->uip->city->text() + "\n\
ADDRESS-STATE-PROVINCE: " + ch->uip->state->text() + "\n\
ADDRESS-COUNTRY: " + ch->uip->country->text() + "\n\
OFFTIME: " + ch->uip->offtime->text() + "\n\
SOAPBOX: " + ch->uip->soapbox->toPlainText() + "\n");
    // qDebug() << header;
    f->write(header.toLocal8Bit(), header.length());
}

bool cabrillo_footer(QFile *f) {
    QString foot = "END-OF-LOG:\n";
    // qDebug() << foot;
    f->write(foot.toLocal8Bit(),foot.length());
}

bool cabrillo_record(QFile *f, int qsonr) {
    QString callsign =  uip->mycall->text().toUpper();
    QString cabritags = "QSO:";
    QStringList cfields = {"freq", "mode", "qso_date", "time_on", "*MYCALL*", "rst_sent", "stx_string", "call", "rst_rcvd", "srx_string", "*T*"   };
    int field_lens[] = {5, 2, 10, 4, 13, 3, 6, 13, 3, 6, 1};

    int j = 0;
    QString val;
    for (auto i = cfields.begin(), end = cfields.end(); i != end; ++i, j++) {

        if ( *i == "*MYCALL*" ) {
            val = callsign;
        } else if ( *i == "*T*" ) {
            val = " ";
        } else {
            QString sqry("select * from qsod where nr=%1 and field='%2';");
            QSqlQuery qry(sqry.arg(qsonr).arg(*i));
            // qDebug() << sqry.arg(qsonr).arg(*i);
            while(qry.next()) {

                val = qry.value(2).toString();

                if ( cfields.at(j) == "freq" ) {
                    bool ok;
                    float f;
                    f = val.toFloat(&ok);
                    if ( ok && f < 30 ) {
                        long fl = f * 1000;
                        val = QString("%1").arg(fl);
                    }
                }
                else if ( cfields.at(j) == "mode" ) {
                    // CW PH FM RY DG
                    if ( val.startsWith("ft") )
                        val =  "DG";
                    else if ( val == "rtty")
                        val =  "RT";
                    else if ( val == "asci")
                        val =  "RT";
                    else if ( val == "usb")
                        val =  "SSB";
                    else if ( val == "lsb")
                        val =  "SSB";
                    else if ( val.contains("fsk") )
                        val =  "DG";
                    else if ( val == "sstv")
                        val =  "DG";
                    else if ( val == "olivia")
                        val =  "DG";
                    else
                        val = "PH";
                } else if ( cfields.at(j) == "qso_date" ) {
                    val.insert(4, '-');
                    val.insert(7, '-');
                }
            }

        }
        val.truncate(field_lens[j]);
        cabritags.append(" " + val.leftJustified(field_lens[j]));
        // qDebug() << *i << val << field_lens[j];
    }
    cabritags.append("\n");
    // qDebug() << cabritags;
    f->write(cabritags.toLocal8Bit(), cabritags.length());
}

bool export_cabrillo(QFile *fi, cabrilloheader *ch) {
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
    cabrillo_header(fi, ch);
    QModelIndexList range = select->selectedRows(); // return selected row(s)
    for(int i = 0; i < range.length(); i++) {
        //        qDebug() << range.at(i).row() << " : " << range.at(i).data().toInt();
        cabrillo_record(fi, range.at(i).data().toInt() );
    }
    cabrillo_footer(fi);
    fi->close();
    return true;
}

void savefield( QComboBox *cb, QString name ) {
    conf.setValue(name, cb->currentText());
}
void savefield( QLineEdit *le, QString name ) {
    conf.setValue(name, le->text());
}
void savefield( QTextEdit *te, QString name ) {
    conf.setValue(name, te->toPlainText());
}
void cabrillo_savetoconfig(cabrilloheader *ch) {
    savefield(ch->uip->address, "ch-address");
    savefield(ch->uip->assisted, "ch-assisted");
    savefield(ch->uip->bands, "ch-bands");
    savefield(ch->uip->callsign, "ch-callsign");
    savefield(ch->uip->catop, "ch-catop");
    savefield(ch->uip->certificate, "ch-certificate");
    savefield(ch->uip->city, "ch-city");
    savefield(ch->uip->club, "ch-club");
    savefield(ch->uip->contest, "ch-contest");
    savefield(ch->uip->country, "ch-country");
    savefield(ch->uip->email, "ch-email");
    savefield(ch->uip->location, "ch-location");
    savefield(ch->uip->locator, "ch-locator");
    savefield(ch->uip->mode, "ch-mode");
    savefield(ch->uip->name, "ch-name");
    savefield(ch->uip->offtime, "ch-offtime");
    savefield(ch->uip->operators, "ch-operators");
    savefield(ch->uip->overlay, "ch-overlay");
    savefield(ch->uip->postalcode, "ch-postalcode");
    savefield(ch->uip->power, "ch-power");
    savefield(ch->uip->score, "ch-score");
    savefield(ch->uip->soapbox, "ch-soapbox");
    savefield(ch->uip->state, "ch-state");
    savefield(ch->uip->station, "ch-station");
    savefield(ch->uip->time, "ch-time");
    savefield(ch->uip->trx, "ch-trx");
    conf.sync();
}


void restorefield( QComboBox *cb, QString name ) {
    if ( !conf.value(name, "").toString().isEmpty() )
        cb->setCurrentText( conf.value(name).toString() );
}
void restorefield( QLineEdit *le, QString name ) {
    if ( !conf.value(name,  "").toString().isEmpty() )
        le->setText( conf.value(name).toString() );
}
void restorefield( QTextEdit *te, QString name ) {
    if ( !conf.value(name,  "").toString().isEmpty() )
        te->setText(conf.value(name).toString() );
}
void cabrillo_restorefromconfig(cabrilloheader *ch) {
    restorefield(ch->uip->address, "ch-address");
    restorefield(ch->uip->assisted, "ch-assisted");
    restorefield(ch->uip->bands, "ch-bands");
    restorefield(ch->uip->callsign, "ch-callsign");
    restorefield(ch->uip->catop, "ch-catop");
    restorefield(ch->uip->certificate, "ch-certificate");
    restorefield(ch->uip->city, "ch-city");
    restorefield(ch->uip->club, "ch-club");
    restorefield(ch->uip->contest, "ch-contest");
    restorefield(ch->uip->country, "ch-country");
    restorefield(ch->uip->email, "ch-email");
    restorefield(ch->uip->location, "ch-location");
    restorefield(ch->uip->locator, "ch-locator");
    restorefield(ch->uip->mode, "ch-mode");
    restorefield(ch->uip->name, "ch-name");
    restorefield(ch->uip->offtime, "ch-offtime");
    restorefield(ch->uip->operators, "ch-operators");
    restorefield(ch->uip->overlay, "ch-overlay");
    restorefield(ch->uip->postalcode, "ch-postalcode");
    restorefield(ch->uip->power, "ch-power");
    restorefield(ch->uip->score, "ch-score");
    restorefield(ch->uip->soapbox, "ch-soapbox");
    restorefield(ch->uip->state, "ch-state");
    restorefield(ch->uip->station, "ch-station");
    restorefield(ch->uip->time, "ch-time");
    restorefield(ch->uip->trx, "ch-trx");
}
