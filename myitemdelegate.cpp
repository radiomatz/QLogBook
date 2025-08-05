#include "myitemdelegate.h"
#include <QLineEdit>
#include <QSqlTableModel>

extern QSqlTableModel *mtvd;


MyItemDelegate::MyItemDelegate(QObject *parent) : QStyledItemDelegate{parent} {
}

QWidget *MyItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const {
    QLineEdit *qle = new QLineEdit(parent);
    return(qle);
}

void MyItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const  {
    ((QLineEdit*)editor)->setText(index.data().toString());
    qDebug() << "set data:" << index.data().toString();
}

void MyItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                  const QModelIndex &index) const  {
    if ( index.isValid() ) {
        QLineEdit *qle = (QLineEdit*)editor;
        QString value = qle->text();
        qDebug() << "value:" << value;
        model->setData(index, value, Qt::EditRole);
        editor->close();
    }
}

void MyItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const  {
    ((QLineEdit*)editor)->setGeometry(option.rect);
    ((QLineEdit*)editor)->updateGeometry();
}
