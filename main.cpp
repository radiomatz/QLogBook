#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QMessageBox>

#include "QLogBook.h"
#include <getopt.h>
#include <QSettings>
#include <QLineEdit>
#include <QSettings>
#include "mliste.h"
#include <QListView>

QString dbpath = ".", exportpath = ".";

QSqlDatabase db;
QSqlTableModel *mtv, *mtvd;
ADI adi;
MainWindow *pw;
QString mycall = "N0CALL", mygrid = "AA00", host = "", browser, browserargs;
int port = 0;
QSettings conf(MY_ORG,MY_PROG);
QProcess *process;
bool debugsql = false;
Ui::MainWindow *uip;
Worker *worker;

int main(int argc, char *argv[]) {
int opt = 0, do_import_adif = 0;
char *fnimport = NULL;

    while ((opt = getopt (argc, argv, "i:")) != -1) {
        switch (opt) {
        case 'i':
            fnimport = optarg;
            do_import_adif = 0; // TODO: if this is active, Debugger is not running anymore
            break;
        }
    }

    QApplication a(argc, argv);


    if ( a.arguments().contains("-i"))
        do_import_adif = 1;

    QCoreApplication::setOrganizationName(MY_ORG);
    QCoreApplication::setOrganizationDomain(MY_DOMAIN);
    QCoreApplication::setApplicationName(MY_PROG);

    getconf();

    if ( !open_db() )
        exit(1);
    create_table_if_not_exist();

    if ( do_import_adif && fnimport != NULL ) {
        int nr = import_adif(fnimport);
        qDebug() << "read " << nr << " QSOs from " << fnimport;
        exit(0);
    }



    MainWindow w;
    pw = &w;
    process = new QProcess(pw);
    w.show();

    update_fields();

    mListe *pmliste = new mListe();
    QListView *qv = pw->findChild<QListView*>("adifields");
    qv->setModel(pmliste);

    generate_qsolist(&w);

    return a.exec();
}
