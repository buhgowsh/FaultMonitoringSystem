#pragma once
#include <QAbstractTableModel>
#include "processlister.h"

class ProcessModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit ProcessModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

public slots:
    void refresh();

private:
    QVector<ProcessInfo> m_procs;
};