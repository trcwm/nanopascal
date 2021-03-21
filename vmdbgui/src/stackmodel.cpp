
#include <QStringList>
#include <QVariant>
#include <QColor>
#include "stackmodel.h"

StackModel::StackModel(VMWrapper *vm, QObject *parent) : m_vm(vm), QAbstractTableModel(parent)
{
}

int StackModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return memsize;
}

int StackModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 2;
}

void StackModel::update()
{
    beginResetModel();
    endResetModel();
}

QVariant StackModel::data(const QModelIndex &index, int role) const
{
    if (m_vm == nullptr)
    {
        return QVariant();
    }

    if (!index.isValid())
    {
        return QVariant();
    }

    if (index.row() >= memsize)
    {
        return QVariant();
    }

    if (role == Qt::BackgroundColorRole)
    {
        if (index.row() == m_vm->m_context->t)
        {
            return QColor(128,128,128);
        }
        else if (index.row() == m_vm->m_context->b)
        {
            return QColor(128,255,255);
        }
        else
        {
            return QColor(255,255,255);
        }
    }
    else if (role == Qt::DisplayRole)
    {
        QString numStr;
        switch(index.column())
        {
        case 0:
            return QStringLiteral("0x%1").arg(index.row(), 4, 16, QLatin1Char('0'));
        case 1:
            return QStringLiteral("0x%1\t%2").arg(m_vm->m_context->dstack[index.row()], 4, 16, QLatin1Char('0')).arg(m_vm->m_context->dstack[index.row()]);
        default:
            break;        
        }
    }


    return QVariant();
}

QVariant StackModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) 
    {
        switch(section)
        {
        case 0:
            return QString("Address");
        case 1:
            return QString("Value");
        default:
            return QVariant();
        }
    }
    return QVariant();
}
