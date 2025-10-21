#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QModelIndex>
#include <QAbstractButton>
#include "worker.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QString submode = "";


private slots:
    void sigfreq(long f);
    void sigmode(QString m);
    void showmode(QString m);

    bool eventFilter(QObject *watched, QEvent *event) override;

    void on_qsoView_clicked(const QModelIndex &index);
    void on_qsoView_activated(const QModelIndex &index);

    void on_qrzButton_clicked();
    void on_newButton_clicked();
    void on_delButton_clicked();

    void fieldChanged();
    void on_qsodView_doubleClicked(const QModelIndex &index);
    void removeQsoDetail();
    void on_qsodView_customContextMenuRequested(const QPoint &pos);
    void on_qsodView_clicked(const QModelIndex &index);

    void on_adifields_doubleClicked(const QModelIndex &index);

    void mysubmit();

    void on_actionSettings_triggered();
    void on_actionImport_ADIF_triggered();
    void on_actionExport_ADIF_triggered();



    void on_actionExport_CABRILLO_triggered();

private:
    Ui::MainWindow *ui;

    int currRow = -1;
    int currdRow = -1;
    QString field = "", oldval = "";

};
#endif // MAINWINDOW_H
