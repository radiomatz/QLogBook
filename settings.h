#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <QAbstractButton>
#include <QSettings>


namespace Ui {
class Settings;
}

class Settings : public QDialog
{
    Q_OBJECT

public:
    explicit Settings(QWidget *parent = nullptr);
    ~Settings();
    void addfld();
    void getfld();

private slots:
    void on_bselect_clicked();
    void on_bexportpath_clicked();
    void on_bokcancel_accepted();
    void on_bokcancel_rejected();


private:
    Ui::Settings *ui;
};

#endif // SETTINGS_H
