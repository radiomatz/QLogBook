#include "settings.h"
#include "ui_settings.h"
#include <QFileDialog>
#include "QLogBook.h"
#include <QDir>
#include <ui_settings.h>
#include <QTableWidget>
#include <QMessageBox>

QStringList additionalvalues(NRADIFIELDS);

Settings::Settings(QWidget *parent) : QDialog(parent), ui(new Ui::Settings) {
    ui->setupUi(this);
    QHeaderView* header = ui->addfields->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Stretch);
    getconf();
    ui->edbpath->setText(dbpath);
    ui->emycall->setText(mycall);
    ui->host->setText(host);
    ui->port->setText(QString(QString("%1").arg(port)));
    ui->browser->setText(browser);
    ui->browserargs->setText(browserargs);
    ui->dontaskfordelete->setChecked( (
        conf.value("dontaskfordelete").toInt() == 1 ? true : false) );
    ui->exportpath->setText(exportpath);
    ui->backupdb->setChecked(conf.value("dbbackup").toInt() == 1 ? true : false);
    addfld();
}


Settings::~Settings() {
    delete ui;
}

void Settings::addfld() {
    ui->addfields->setColumnCount(2);
    ui->addfields->setRowCount(NRADIFIELDS);
    for ( int i = 0; i < NRADIFIELDS; i++ ) {
        ui->addfields->setItem(i, 0, new QTableWidgetItem(adif_fields[i]));
        ui->addfields->setItem(i, 1, new QTableWidgetItem(additionalvalues[i]));
    }
}

void Settings::getfld() {
    QTableWidgetItem *it;
    for ( int i = 0; i < NRADIFIELDS; i++ ) {
        it = ui->addfields->item(i,1);
        QString txt = it->text();
        additionalvalues[i] = txt.toLocal8Bit();
    }
}

bool getconf() {
    dbpath = conf.value("dbpath", dbpath).toString();
    exportpath = conf.value("exportpath", exportpath).toString();
    mycall = conf.value("mycall", mycall).toString();
    host = conf.value("host", "").toString();
    port = conf.value("port", 0).toInt();
    browser = conf.value("browser", "firefox").toString();
    browserargs = conf.value("browserargs", "").toString();

    QStringList tmp = conf.value("additionalvalues", additionalvalues).toStringList();
    if ( tmp.length() == additionalvalues.length() ) {
        additionalvalues = tmp;
    }
    else {
        QMessageBox msgBox(QMessageBox::Information, "Read Settings", "Additional ADIF Fields are lost because of Change", QMessageBox::Ok);
        msgBox.exec();
    }
    return true;
}



void Settings::on_bselect_clicked() {
    QString dirName = QFileDialog::getExistingDirectory(this, "Database Folder", dbpath);
    if ( !dirName.isEmpty()) {
        // qDebug() << dirName;
    }
    dbpath = dirName;
    ui->edbpath->setText(dirName);
}
void Settings::on_bexportpath_clicked() {
    QString dirName = QFileDialog::getExistingDirectory(this, "Expor Adif Folder", dbpath);
    if ( !dirName.isEmpty()) {
        // qDebug() << dirName;
    }
    exportpath = dirName;
    ui->exportpath->setText(dirName);
}

void Settings::on_bokcancel_accepted() {
    mycall = ui->emycall->text().toUpper();
    uip->mycall->setText(mycall);
    conf.setValue("mycall", mycall);

    QString olddbpath = dbpath;
    dbpath = ui->edbpath->text();

    conf.setValue("dbpath", dbpath);
    exportpath = ui->exportpath->text();
    conf.setValue("exportpath", exportpath);

    host = ui->host->text();
    conf.setValue("host", host);

    port = ui->port->text().toInt();
    conf.setValue("port", port);

    browser = ui->browser->text();
    conf.setValue("browser", browser);

    browserargs = ui->browserargs->text();
    conf.setValue("browserargs", browserargs);

    getfld();
    conf.setValue("additionalvalues", additionalvalues);

    conf.setValue("dontaskfordelete", (ui->dontaskfordelete->isChecked()?1:0) );
    conf.setValue("dbbackup", (ui->backupdb->isChecked()?1:0) );

    QString tmpdbpath = ui->edbpath->text();
    if ( tmpdbpath.compare(dbpath) != 0 || olddbpath.compare(dbpath) != 0 ) {
        conf.setValue("dbpath", tmpdbpath);
        if ( !open_db() ) {
            this->reject();
        } else {
            create_table_if_not_exist();
        }
    }
}


void Settings::on_bokcancel_rejected() {
}




