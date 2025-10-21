#ifndef CABRILLOHEADER_H
#define CABRILLOHEADER_H

#include <QDialog>

namespace Ui {
class cabrilloheader;
}

class cabrilloheader : public QDialog
{
    Q_OBJECT

public:
    explicit cabrilloheader(QWidget *parent = nullptr);
    ~cabrilloheader();
    Ui::cabrilloheader *uip;
    bool ready = false;

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void prepare();

private:
    Ui::cabrilloheader *ui;
};

#endif // CABRILLOHEADER_H
