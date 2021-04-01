
#include <QStringList>
#include <QVariant>
#include <QColor>
#include "instrmodel.h"

InstrModel::InstrModel(VMWrapper *vm, QObject *parent) : m_vm(vm), QAbstractTableModel(parent)
{
}

int InstrModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    if (m_vm == nullptr)
        return 0;

    return m_vm->m_context->memsize / sizeof(instruction_t);
}

int InstrModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 3;
}

QVariant InstrModel::data(const QModelIndex &index, int role) const
{
    if (m_vm == nullptr)
    {
        return QVariant();
    }

    if (!index.isValid())
    {
        return QVariant();
    }

    if (index.row() >= (m_vm->m_context->memsize / sizeof(instruction_t)))
    {
        return QVariant();
    }

    if (role == Qt::BackgroundColorRole)
    {
        if (index.row() == m_vm->m_context->pc)
        {
            return QColor(128,128,128);
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
        {
            const uint16_t pc = index.row();
            const instruction_t* ins = (const instruction_t*)&m_vm->m_context->mem[pc*sizeof(instruction_t)];
            return QStringLiteral("0x%1  0x%2").arg(ins->opcode, 2, 16, QLatin1Char('0'))
                .arg(ins->opt16, 4, 16,QLatin1Char('0'));            
        }
        case 2:
            return disasm(index.row());
        default:
            break;        
        }
    }


    return QVariant();
}

QVariant InstrModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) 
    {
        switch(section)
        {
        case 0:
            return QString("Address");
        case 1:
            return QString("Opcode");
        case 2:
            return QString("Instruction");
        default:
            return QVariant();
        }
    }
    return QVariant();
}

void InstrModel::update()
{
    beginResetModel();
    endResetModel();
}

QString decodealu(uint16_t imm16)
{
    switch(imm16)
    {
    case OPR_RET:
        return QString("RET");
        
    case OPR_NEG:
        return QString("NEG");
        
    case OPR_ADD:    
        return QString("ADD");
        
    case OPR_SUB:    
        return QString("SUB");
            
    case OPR_MUL:    
        return QString("MUL");
                    
    case OPR_DIV:    
        return QString("DIV");
                    
    case OPR_ODD:    
        return QString("ODD");
        
    case OPR_EQ:
        return QString("EQU");
                           
    case OPR_NEQ:
        return QString("NEQ");
                     
    case OPR_LESS:
        return QString("LES");
             
    case OPR_LEQ:
        return QString("LEQ");
             
    case OPR_GREATER:
        return QString("GRE");
             
    case OPR_GEQ:
        return QString("GEQ");

    case OPR_SHL:
        return QString("SHL");

    case OPR_SHR:
        return QString("SHR");

    case OPR_SAR:
        return QString("SAR");

    case OPR_OUTCHAR:
        return QString("OUTCHAR");

    case OPR_OUTINT:
        return QString("OUTINT");

    case OPR_INCHAR:
        return QString("OUTCHAR");

    case OPR_ININT:
        return QString("ININT");

    default:
        return QString("???\n");
        
    }
}

QString InstrModel::disasm(uint16_t pc) const
{
    const instruction_t* ins = (const instruction_t*)&m_vm->m_context->mem[pc*sizeof(instruction_t)];
    uint8_t     opcode = ins->opcode;
    uint16_t    imm16  = ins->opt16;
    switch (opcode & 0xF)
    {
    case VM_LIT:
        return QString::asprintf("LIT\t%d", imm16);

    case VM_OPR:
        return decodealu(imm16);

    case VM_LOD:
        return QString::asprintf("LOD\tlvl:%d\tofs:%d", opcode >> 4 ,imm16);
        
    case VM_STO:
        return QString::asprintf("STO\tlvl:%d\tofs:%d", opcode >> 4 ,imm16);
        
    case VM_CAL:
        return QString::asprintf("CAL\tlvl:%d\t0x%04X", opcode >> 4, imm16);
        
    case VM_INT:
        return QString::asprintf("INT\t%d", imm16);
        
    case VM_JMP:
        return QString::asprintf("JMP\t0x%04X", imm16);
        
    case VM_JPC:
        return QString::asprintf("JPC\t0x%04X", imm16);
        
    case VM_HALT:
        return QString::asprintf("HALT");

    case VM_LODX:
        return QString::asprintf("LODX\tlvl:%d\tofs:%d", opcode >> 4 ,imm16);
        
    case VM_STOX:
        return QString::asprintf("STOX\tlvl:%d\tofs:%d", opcode >> 4 ,imm16);

    default:
        return QString::asprintf("???\n");
        
    }
}