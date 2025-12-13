#include "mliste.h"
#include "QLogBook.h"

mListe::mListe(QObject *parent)
    : QAbstractTableModel{parent} {

    // make array for values of adiffields equal in size
    adi.f.fill("", adif_fields.length());
}

int mListe::rowCount(const QModelIndex & /*parent*/) const
{
    return NRADIFIELDS;
}

int mListe::columnCount(const QModelIndex & /*parent*/) const
{
    return 1;
}

QVariant mListe::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
        return QString(adif_fields[index.row()]);

    return QVariant();
}
