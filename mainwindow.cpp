#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settings.h"
#include "cabrilloheader.h"
#include "grids.h"

#include "QLogBook.h"
#include <QFileDialog>
#include <QAction>
#include <QKeySequence>
#include <QKeyEvent>
#include <QTreeWidget>
#include "worker.h"
#include <QMessageBox>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    uip = ui;
    sigfreq(conf.value("lastfreq", 0).toFloat());
    connect(
        ui->qsodView->itemDelegate(),
        SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)),
        SLOT(mysubmit()));

    QThread *thread = new QThread;
    worker = new Worker();
    worker->moveToThread(thread);
    /*
    if ( !host.isEmpty() && !port == 0 ) {
        worker->tcp = new QTcpSocket();
        worker->tcp->connectToHost(host, port);
        if ( !worker->tcp->isOpen() ) {
            qDebug() << "TCP connect failed! Port or Host wrong? YourPort:" << port << "YourHost:" << host;
        }
    }
    */
    connect(worker, SIGNAL(sigfreq(long)), this, SLOT(sigfreq(long)));
    connect(worker, SIGNAL(sigmode(QString)), this, SLOT(sigmode(QString)));
    connect(thread, SIGNAL(started()), worker, SLOT(process()));
    connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
    connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
    thread->start();
    connect(ui->qsodView->itemDelegate(), SIGNAL(closeEditor(QWidget*, QAbstractItemDelegate::EndEditHint)), SLOT(fieldChanged()));

    ui->qsoView->installEventFilter(this);

}


MainWindow::~MainWindow() {
    worker->quit(); // Hamblib
    if ( worker->tcp->isOpen() )
        worker->tcp->close();
    process->kill(); // Browser Process
    conf.setValue("lastfreq", ui->freq->text().replace(".",""));
    delete ui;
}




void MainWindow::sigfreq(long f) {
    if ( f != 0 ) {
        QString tmp = QString("%1").arg(f);
        while ( tmp.length() < 12 )
            tmp = tmp.insert(0, " ");
        ui->freq->setText(tmp);
//        if ( f == 14230000 || f == 21340000 || f == 28680000 )
//            ui->cmode->setCurrentText("SSTV");
    }
}

void MainWindow::showmode(QString m) {
    if ( m.startsWith("RPRT") )
        return;

// why this? because only automatic setted modes should be overwritten automatic
    if (
        ui->cmode->currentText() == "FM" ||
        ui->cmode->currentText() == "AM" ||
        ui->cmode->currentText() == "SSB" ||
        ui->cmode->currentText() == "CW" ||
        ui->cmode->currentText() == "RTTY" ||
        ui->cmode->currentText() == "PKT"
        )
        ui->cmode->setCurrentText(m);
}

void MainWindow::sigmode(QString m) {
    if ( m.startsWith("RPRT"))
        return; // got error from XMT

    // why this? because only automatic setted modes should be overwritten automatic
    submode = m.remove("\n");
    if ( submode.startsWith("FM") )
        showmode("FM");
    else if ( submode == "AM" )
        showmode("AM");
    else if (submode == "USB" || submode == "LSB" )
        showmode("SSB");
    else if ( submode.startsWith("CW") )
        showmode("CW");
    else if ( submode.startsWith("RTTY") )
        showmode("RTTY");
    else if ( submode.startsWith("PKT") )
        showmode("PKT");
}



/*
 * QSO List
 */
void MainWindow::showdistance(void) {
    int rg = findrow("gridsquare");
    QString mdh;
    if ( rg >= 0 )
        mdh = mtvd->index(rg, 2).data().toString();
    if ( mdh.length() >= 4 ) {
        float lon, lat, mylon, mylat;
        grids_maidenhead2latlon(mygrid, &mylat, &mylon);
        grids_maidenhead2latlon(mdh, &lat, &lon);
        int dist = grids_dist(mylat, mylon, lat, lon);
        // qDebug() << "Call:" << mtvd->index(ic, 2).data().toString() << "Loc:" << mdh << "Dist:" << dist << "lat:" << lat << "lon:" << lon;
        ui->dist->setText(QString("%1 km").arg(dist));
    } else
        ui->dist->setText("");
}

void MainWindow::on_qsoView_clicked(const QModelIndex &index) {
    int qsonr = mtv->index(index.row(), 0).data().toInt();
    generate_qsodetaillist(this, qsonr);
    showdistance();
}

void MainWindow::on_qsoView_activated(const QModelIndex &index) {
    on_qsoView_clicked(index);
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
    // TODO: make QSqlTableView do this itself!
    if (watched == ui->qsoView && event->type() == QEvent::KeyPress) {
        QKeyEvent *kev = static_cast<QKeyEvent*>(event);
        if ( kev->key() == Qt::Key_Down || kev->key() == Qt::Key_Up ) {
            int row = ui->qsoView->currentIndex().row();
            if ( kev->key() == Qt::Key_Down ) {
                if ( row < ( mtv->rowCount() - 1 ) )
                    row++;
            }
            if ( kev->key() == Qt::Key_Up ) {
                if ( row > 0 )
                    row--;
            }
            mtv->selectRow(row);
            int qsonr = mtv->index(row, 0).data().toInt();
            generate_qsodetaillist(this, qsonr);
            // qDebug() << "row:" << row << "cnt:" << mtv->rowCount();
            showdistance();
        }
    }
    return false;
}







/*
 * general Commit of database changes
 */



void MainWindow::mysubmit() {
    mtvd->submitAll();
    mtv->select();
}



/*
 *  QSO Detail View Handling
 */

int findrow(QString what) {
    int n = mtvd->rowCount();
    for ( int i = 0; i < n; i++ ) {
        // qDebug() << mtvd->index(i, 1).data().toString();
        if ( what.compare(mtvd->index(i, 1).data().toString(), Qt::CaseInsensitive) == 0 ) {
            return(i);
        }
    }
    return(-1);
}

void MainWindow::fieldChanged() {
    QString newval = mtvd->index(currRow, 2).data().toString();

    // qDebug() << "field" << field << "changed from" << oldval << "to" << newval;

    if ( field.compare("band", Qt::CaseInsensitive) == 0 ) {

    } else if ( field.compare("freq", Qt::CaseInsensitive) == 0 ) {
        QString sfreq = newval.replace(".","");
        float freq = sfreq.toDouble();
        freq /= 1000.0F;
        QString band = find_band(freq);
        int rb = findrow("band");
        if ( rb >= 0 ) {
            mtvd->setData(mtvd->index(rb, 2), band);
            mtvd->submitAll();
        }
    }
}

void MainWindow::on_qsodView_doubleClicked(const QModelIndex &index) {
    // qDebug() << " double click:" << index.row();
    currRow = index.row();
    field =  mtvd->index(currRow, 1).data().toString();
    oldval = mtvd->index(currRow, 2).data().toString();
}

void MainWindow::removeQsoDetail() {
    mtvd->removeRow(currdRow);
    mtvd->submitAll();
}

void MainWindow::on_qsodView_customContextMenuRequested(const QPoint &pos){
    // needs Attr ownContextMenu in ui
    QModelIndex cur = ui->qsodView->indexAt(pos);
    currdRow = cur.row();
    QMenu contextMenu("DetailMenu", ui->qsodView);
    QAction action1(QString("Remove %1:%2 ?").arg(mtvd->index(currdRow, 1).data().toString()).arg(mtvd->index(currdRow, 2).data().toString()), ui->qsodView);
    connect(&action1, SIGNAL(triggered()), this, SLOT(removeQsoDetail()));
    contextMenu.addAction(&action1);
    contextMenu.exec(ui->qsodView->viewport()->mapToGlobal(pos));
}

void MainWindow::on_qsodView_clicked(const QModelIndex &index) {
}



/*
 *  ADIF Frame
 */



void MainWindow::on_adifields_doubleClicked(const QModelIndex &index) {
    int qsonr = mtvd->index(0,0).data().toInt();
    if ( qsonr == 0 ) {
        return;
    }
    QString qry  = "insert into qsod(nr,field) values(%1,'%2');";
    doquery(qry.arg(qsonr).arg(adif_fields[index.row()]));
    mtvd->select();
}




/*
 *  Buttons
 */


void MainWindow::on_qrzButton_clicked() {
    QString callsign = this->findChild<QLineEdit *>("inp_callsign")->text().toUpper();
    show_qrz_window(callsign);
}

void MainWindow::on_delButton_clicked() {
    QModelIndexList qmi = ui->qsoView->selectionModel()->selectedRows();
    if ( delete_qso(qmi) ) {
        mtv->select();
        mtvd->select();
    }
}

void MainWindow::on_newButton_clicked() {
    QString callsign = this->findChild<QLineEdit *>("inp_callsign")->text().toUpper();
    if ( make_new_qso(callsign) ) {
        mtv->select();
        mtvd->select();
        increment_counter();
    }
}



/*
 *  Menu
 */

void MainWindow::on_actionSettings_triggered() {
    Settings *conf = new Settings();
    conf->show();
}


void MainWindow::on_actionImport_ADIF_triggered() {
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Open File", "$HOME/", "Adif Files (*.adi *.adif)");
    if ( !fileName.isEmpty()) {
        import_adif(fileName.toLocal8Bit().data());
        mtv->select();
    }
}


void MainWindow::on_actionExport_ADIF_triggered() {
    QDateTime local(QDateTime::currentDateTime());
    QDateTime UTC(local.toUTC());

    QString dt = UTC.toString("yyyyMMdd"); // .toString("yyyy-MM-dd hh:mm:ss");
    QString tm = UTC.toString("hhmmss");
    QString fname(exportpath + QDir::separator() + "export-"  + dt + "-" + tm + ".adif");

    QFile fi(fname);

    if ( export_adif(&fi) ) {
        QMessageBox msgBox(QMessageBox::Information, "Export Adif", QString("Adif exported to: ") + fname, QMessageBox::Ok);
        msgBox.exec();
    }

    return;
}


void MainWindow::on_actionExport_CABRILLO_triggered() {

    QItemSelectionModel *select = uip->qsoView->selectionModel();
    if ( !select->hasSelection() ) {
        QMessageBox msgBox(QMessageBox::Critical, "No Range", "Nothing to Export selected", QMessageBox::Abort);
        msgBox.exec();
        return;
    }

    cabrilloheader *ch = new cabrilloheader();
    cabrillo_restorefromconfig(ch);
    if ( ch->exec() != QDialog::Accepted )
        return;
    cabrillo_savetoconfig(ch);

    QDateTime local(QDateTime::currentDateTime());
    QDateTime UTC(local.toUTC());

    QString dt = UTC.toString("yyyyMMdd"); // .toString("yyyy-MM-dd hh:mm:ss");
    QString tm = UTC.toString("hhmmss");
    QString fname(exportpath + QDir::separator() + "export-"  + dt + "-" + tm + ".cbr");

    QFile fi(fname);

    if ( export_cabrillo(&fi, ch) ) {
        QMessageBox msgBox(QMessageBox::Information, "Export Cabrillo", QString("Cabrillo exported to: ") + fname, QMessageBox::Ok);
        msgBox.exec();
    }

    return;

}


void MainWindow::on_actionExport_QRZ_triggered() {
    QItemSelectionModel *select = uip->qsoView->selectionModel();
    if ( !select->hasSelection() ) {
        QMessageBox msgBox(QMessageBox::Critical, "No Range", "Nothing to Export selected", QMessageBox::Abort);
        msgBox.exec();
        return;
    }

    QString log = export_qrz(select);
    if ( log.length() > 0 ) {
        QMessageBox msgBox(QMessageBox::Information, "Export to QRZ Site", log, QMessageBox::Ok);
        msgBox.exec();
    }

    return;
}

