#ifndef MLISTE_H
#define MLISTE_H

#include <QAbstractTableModel>

class mListe : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit mListe(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;};

#endif // MLISTE_H
