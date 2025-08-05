#ifndef QLOGBOOK_H

#include <QtSql/qtsqlglobal.h>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QWidget>
#include <QTableView>
#include <QSqlTableModel>
#include <QMainWindow>
#include "mainwindow.h"
#include <QtSql/QSqlQuery>
#include <QModelIndex>
#include <stdint.h>
#include <QSettings>
#include <QProcess>
#include <QFile>
#include <QTcpSocket>
#include "ui_mainwindow.h"

#define MY_PROG "QLogBook"
#define MY_VERSION "0.1.1"
#define MY_ORG "DM2HR"
#define MY_DOMAIN "dm2hr.hmro.de"

#define NRADIFIELDS 180
#define ADIFIELDSIZE 25

extern QSqlDatabase db;
extern QSqlTableModel *mtv, *mtvd;
extern MainWindow *pw;
extern QString dbpath, exportpath, host, mycall, browser, browserargs;
extern int port;
extern QSettings conf;
extern QProcess *process;
extern Ui::MainWindow *uip;
extern bool debugsql;
extern Worker* worker;

#define EXPORTFILE "qlogbook-export.adif"

bool open_db();
bool generate_qsolist(MainWindow *parent);
bool generate_qsodetaillist(MainWindow *parent, int nr);
bool show_qrz_window(QString call);
bool make_new_qso(QString callsign);
bool delete_qso(QModelIndexList qmi);
bool handle_field(const QModelIndex &index);
bool getconf();
bool update_fields();
long HamlibFreq();
QString HamlibMode();
bool increment_counter();
QString find_band ( float freq );

struct sbands { char *mtr; float lower; float upper; };

extern char *adif_fields[NRADIFIELDS];
struct ADIFRECORD {
//    db_recno_t nr;
    uint8_t f[NRADIFIELDS][ADIFIELDSIZE];
}  __attribute__((aligned(1),packed));
typedef struct ADIFRECORD ADI;
extern ADI adi;

int import_adif(char *filename);
bool export_adif(QFile *fi = new QFile(EXPORTFILE));
int write_adif_record();
int find_adif_field(char *what);
bool doquery(QString qsz);

int create_table_if_not_exist();

#define QLOGBOOK_H
#endif // QLOGBOOK_H
