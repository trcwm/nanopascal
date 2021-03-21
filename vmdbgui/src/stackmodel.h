#pragma once

#include <QAbstractTableModel>
#include "vmwrapper.h"

class StackModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    StackModel(VMWrapper *vm, QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

protected:
    VMWrapper *m_vm;
};
