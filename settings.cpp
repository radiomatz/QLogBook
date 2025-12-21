#include "settings.h"
#include "ui_settings.h"
#include <QFileDialog>
#include "QLogBook.h"
#include <QDir>
#include <ui_settings.h>
#include <QTableWidget>
#include <QMessageBox>

int min(int a, int b) {
    return(a>b?b:a);
}


QStringList additionalfields;
QStringList additionalvalues;

Settings::Settings(QWidget *parent) : QDialog(parent), ui(new Ui::Settings) {
    ui->setupUi(this);
    QHeaderView* header = ui->addfields->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Stretch);
    getconf();
    ui->edbpath->setText(dbpath);
    ui->emycall->setText(mycall);
    ui->grid->setText(mygrid);
    ui->host->setText(host);
    ui->port->setText(QString(QString("%1").arg(port)));
    ui->browser->setText(browser);
    ui->browserargs->setText(browserargs);
    ui->dontaskfordelete->setChecked( (
        conf.value("dontaskfordelete").toInt() == 1 ? true : false) );
    ui->exportpath->setText(exportpath);
    ui->backupdb->setChecked(conf.value("dbbackup").toInt() == 1 ? true : false);
    ui->qrzkey->setText(conf.value("qrzkey").toString());
    addfld();
}


Settings::~Settings() {
    delete ui;
}

void Settings::addfld() {
    ui->addfields->setColumnCount(2);
    for ( int i = 0; i < adif_fields.length(); i++ ) {
        if (!additionalfields.contains(adif_fields[i])) {
            additionalfields.append(adif_fields[i]);
            additionalvalues.append("");
        }
    }
    int fl = additionalfields.length();
    ui->addfields->setRowCount(fl + 50); // space for creative people :-))
    for ( int i = 0; i < additionalfields.length() && i < additionalvalues.length(); i++ ) {
        ui->addfields->setItem(i, 0, new QTableWidgetItem(additionalfields[i]));
        ui->addfields->setItem(i, 1, new QTableWidgetItem(additionalvalues[i]));
    }
}

void Settings::getfld() {
    QTableWidgetItem *itf;
    QTableWidgetItem *itv;

    additionalfields.resize(0);
    additionalvalues.resize(0);

    for ( int i = 0; i < ui->addfields->rowCount(); i++ ) {
        itf = ui->addfields->item(i,0);
        if ( itf ) {
            QString txtf = itf->text();
            additionalfields.append(txtf.toLocal8Bit());
        }

        itv = ui->addfields->item(i,1);
        if ( itv ) {
            QString txtv = itv->text();
            additionalvalues.append(txtv.toLocal8Bit());
        }
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
    mygrid = conf.value("mygrid", "AA00").toString();
    // QStringLists.remove() seems to have a problem, so...
    additionalfields.resize(0);
    QStringList tmpf = conf.value("additionalfields").toStringList();
    additionalvalues.resize(0);
    QStringList tmpv = conf.value("additionalvalues").toStringList();
    for ( int i = 0; i < tmpf.length(); i++ ) {
        if ( tmpf[i].length() > 0 ) {
            additionalfields.append(tmpf[i]);
            additionalvalues.append(tmpv[i]);
        }
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

    mygrid = ui->grid->text().toUpper();
    conf.setValue("mygrid", mygrid);

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

    // shrink when no value given
    QStringList tmpf;
    QStringList tmpv;
    for ( int i = 0; i < additionalvalues.length(); i++ ) {
        if ( additionalvalues[i].length() > 0 ) {
            tmpf.append(additionalfields[i]);
            tmpv.append(additionalvalues[i]);
        }
    }
    conf.setValue("additionalfields", tmpf);
    conf.setValue("additionalvalues", tmpv);


    conf.setValue("dontaskfordelete", (ui->dontaskfordelete->isChecked()?1:0) );
    conf.setValue("dbbackup", (ui->backupdb->isChecked()?1:0) );

    conf.setValue("qrzkey", ui->qrzkey->text());

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
