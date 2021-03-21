#include <QStringList>
#include "regmodel.h"

RegModel::RegModel(VMWrapper *vm, QObject *parent) : m_vm(vm), QAbstractTableModel(parent)
{

}

int RegModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 5;
}

int RegModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 2;
}

void RegModel::update()
{
    beginResetModel();
    endResetModel();
}

QVariant RegModel::data(const QModelIndex &index, int role) const
{
    const QStringList regs = {"PC","T","B","ST0","ST1"};

    if (m_vm == nullptr)
    {
        return QVariant();
    }

    if (index.row() >= regs.length())
    {
        return QVariant();
    }

    if (!index.isValid() || role != Qt::DisplayRole) 
    {
        return QVariant();
    }

    QString numStr;
    switch(index.column())
    {
    case 0:
        return regs[index.row()];
    case 1:
        switch(index.row())
        {
        case 0:
            numStr = QStringLiteral("0x%1").arg(m_vm->m_context->pc, 4, 16, QLatin1Char('0'));
            return numStr;
        case 1:
            numStr = QStringLiteral("0x%1").arg(m_vm->m_context->t, 4, 16, QLatin1Char('0'));
            return numStr;
        case 2:
            numStr = QStringLiteral("0x%1").arg(m_vm->m_context->b, 4, 16, QLatin1Char('0'));
            return numStr;
        case 3:
            return m_vm->m_context->dstack[m_vm->m_context->t];
            break;        
        case 4:
            return m_vm->m_context->dstack[m_vm->m_context->t-1];
            break;
        default:
            return QVariant();
        }
    }

    return QVariant();
}

QVariant RegModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) 
    {
        switch(section)
        {
        case 0:
            return QString("Register");
        case 1:
            return QString("Value");
        default:
            return QVariant();
        }
    }
    return QVariant();
}

