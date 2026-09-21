#include "myitemdelegate.h"
#include <QLineEdit>
#include <QSqlTableModel>
#include <QDebug>
#include "QLogBook.h"
#include "QTimeEdit"
#include "QDateEdit"

#include "QLogBook.h"

QString sfield;

MyItemDelegate::MyItemDelegate(QObject *parent) : QStyledItemDelegate{parent} {
}

QWidget *MyItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const {

    const QAbstractItemModel *model = index.model();
    sfield = model->index(index.row(), 1).data().toString();

    if ( sfield.startsWith("TIME_", Qt::CaseInsensitive)) {
        QTimeEdit *qte = new QTimeEdit(parent);
        qte->setTimeZone(QTimeZone::utc());
        qte->setDisplayFormat(QString("HH:mm:ss"));
        return( qte );
    } else if ( sfield.startsWith("qso_date", Qt::CaseInsensitive)) {
        QDateEdit *qde = new QDateEdit(parent);
        qde->setTimeZone(QTimeZone::utc());
        qde->setDisplayFormat(QString("yyyy-MM-dd"));
        return(qde);
    } else {
        return(new QLineEdit(parent));
    }
}

void MyItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const  {

    if ( sfield.startsWith("TIME_", Qt::CaseInsensitive)) {
        ((QTimeEdit*)editor)->setTime(QTime().fromString(index.data().toString(), "HHmmss"));
    } else if ( sfield.startsWith("qso_date", Qt::CaseInsensitive)) {
        ((QDateEdit*)editor)->setDate(QDate().fromString(index.data().toString(), "yyyyMMdd"));
    } else {
        ((QLineEdit*)editor)->setText(index.data().toString());
    }
}

void MyItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                  const QModelIndex &index) const  {

    if ( index.isValid() ) {
        if ( sfield.startsWith("TIME_", Qt::CaseInsensitive)) {
           model->setData(index, ((QTimeEdit*)editor)->time().toString("HHmmss"), Qt::EditRole);
        } else if ( sfield.startsWith("qso_date", Qt::CaseInsensitive)) {
            model->setData(index, ((QDateEdit*)editor)->date().toString("yyyyMMdd"), Qt::EditRole);
       } else {
            model->setData(index, ((QLineEdit*)editor)->text(), Qt::EditRole);
        }
        // db commit
        mtvd->submitAll();
        mtv->select();
    }
    editor->close();
}

void MyItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const  {
        if ( sfield.startsWith("TIME", Qt::CaseInsensitive)) {
            ((QTimeEdit*)editor)->setGeometry(option.rect);
            ((QTimeEdit*)editor)->updateGeometry();
        } else {
            ((QLineEdit*)editor)->setGeometry(option.rect);
            ((QLineEdit*)editor)->updateGeometry();
        }
}
