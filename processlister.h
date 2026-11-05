#pragma once
#include <QString>
#include <QVector>

struct ProcessInfo {
    qint64  pid = 0;
    QString name;
    QString cmdline;
    QString user;
    qint64  rssKb = 0;
};

QVector<ProcessInfo> listProcesses();