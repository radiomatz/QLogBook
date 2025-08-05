#include "callfield.h"
#include "QLogBook.h"
#include <QTableView>
#include <QHeaderView>

CallField::CallField(QWidget *parent) : QLineEdit(parent)
{
}

void CallField::keyPressEvent(QKeyEvent *event)
{
    if ( event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return ) {
        QString callsign = pw->findChild<QLineEdit *>("inp_callsign")->text().toUpper();
        show_qrz_window(callsign);
        return;
    }
    if ( event->key() == '+' ) {
        event->accept();
        uip->newButton->click();
        return;
    }

    QLineEdit::keyPressEvent(event);
    QString callsign = this->text().toUpper();
    QString filter = "";
    if (!callsign.isEmpty()) {
        filter = QString("call like \"%1%%\"").arg(callsign);
    }
    mtv->setFilter(filter);
    mtv->select();
    generate_qsodetaillist(pw, mtv->index(0,0).data().toInt());
}
