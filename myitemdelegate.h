#ifndef MYITEMDELEGATE_H
#define MYITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QObject>
#include <QItemDelegate>
#include <QAbstractItemModel>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QWidget>
#include <QEvent>

class MyItemDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit MyItemDelegate(QObject *parent = nullptr);

protected:

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;

};

#endif // MYITEMDELEGATE_H
