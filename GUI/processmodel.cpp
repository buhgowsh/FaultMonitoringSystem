#include "processmodel.h"

ProcessModel::ProcessModel(QObject *parent) : QAbstractTableModel(parent) {}

int ProcessModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_procs.size();
}

int ProcessModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 5;
}

QVariant ProcessModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid() || role != Qt::DisplayRole)
        return {};
    const ProcessInfo &p = m_procs.at(idx.row());
    switch (idx.column()) {
    case 0: return p.pid;
    case 1: return p.name;
    case 2: return p.user;
    case 3: return p.rssKb;
    case 4: return p.cmdline;
    }
    return {};
}

QVariant ProcessModel::headerData(int section, Qt::Orientation o, int role) const
{
    if (o != Qt::Horizontal || role != Qt::DisplayRole)
        return {};
    static const QStringList headers{"PID", "Name", "User", "Memory (KB)", "Command"};
    return headers.value(section);
}

void ProcessModel::refresh()
{
    beginResetModel();
    m_procs = listProcesses();
    endResetModel();
}