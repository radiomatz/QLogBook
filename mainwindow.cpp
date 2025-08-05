#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settings.h"

#include "QLogBook.h"
#include <QFileDialog>
#include <QAction>
#include <QKeySequence>
#include <QKeyEvent>
#include <QTreeWidget>
#include "worker.h"
#include <QMessageBox>


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
    if ( m.startsWith("RPRT ") )
        return;
// why this? only automatic setted modes should be overwritten automatic
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
    // why this? only automatic setted modes should be overwritten automatic
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



void MainWindow::on_qsoView_clicked(const QModelIndex &index) {
    int row = index.row();
    int qsonr = 0;

    qsonr = mtv->index(row,0).data().toInt();
    generate_qsodetaillist(this, qsonr);

}
void MainWindow::on_qsoView_pressed(const QModelIndex &index) {
    on_qsoView_clicked(index);
}
void MainWindow::on_qsoView_activated(const QModelIndex &index) {
    on_qsoView_clicked(index);
}





int findrow(QString what) {
    int n = mtvd->rowCount();
    for ( int i = 0; i < n; i++ ) {
        qDebug() << mtvd->index(i, 1).data().toString();
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

void MainWindow::mysubmit() {
    mtvd->submitAll();

    mtv->select();
}





// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
void MainWindow::removeQsoDetail() {
    mtvd->removeRow(currdRow);
    mtvd->submitAll();
}
// needs Attr ownContextMenu in ui
void MainWindow::on_qsodView_customContextMenuRequested(const QPoint &pos){
    QModelIndex cur = ui->qsodView->indexAt(pos);
    currdRow = cur.row();
    QMenu contextMenu("DetailMenu", ui->qsodView);
    QAction action1(QString("Remove %1:%2 ?").arg(mtvd->index(currdRow, 1).data().toString()).arg(mtvd->index(currdRow, 2).data().toString()), ui->qsodView);
    connect(&action1, SIGNAL(triggered()), this, SLOT(removeQsoDetail()));
    contextMenu.addAction(&action1);
    contextMenu.exec(ui->qsodView->viewport()->mapToGlobal(pos));
}
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


void MainWindow::on_qsodView_clicked(const QModelIndex &index) {
}


void MainWindow::on_adifields_doubleClicked(const QModelIndex &index) {
    int qsonr = mtvd->index(0,0).data().toInt();
    if ( qsonr == 0 ) {
        return;
    }
    QString qry  = "insert into qsod(nr,field) values(%1,'%2');";
    doquery(qry.arg(qsonr).arg(adif_fields[index.row()]));
    mtvd->select();
}






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
