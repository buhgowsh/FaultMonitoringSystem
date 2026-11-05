#include "processlister.h"
#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <pwd.h>

static QString readSmallFile(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return {};
    return QString::fromUtf8(f.readAll());
}

QVector<ProcessInfo> listProcesses()
{
    QVector<ProcessInfo> result;
    const auto entries = QDir("/proc").entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString &entry : entries) {
        bool ok = false;
        const qint64 pid = entry.toLongLong(&ok);
        if (!ok)
            continue;                         // not a PID directory

        const QString base = "/proc/" + entry;
        ProcessInfo p;
        p.pid  = pid;
        p.name = readSmallFile(base + "/comm").trimmed();
        if (p.name.isEmpty())
            continue;                         // process exited mid-scan

        p.cmdline = readSmallFile(base + "/cmdline")
                        .replace(QChar('\0'), QChar(' ')).trimmed();

        const QString status = readSmallFile(base + "/status");
        for (const QString &line : status.split('\n')) {
            if (line.startsWith("Uid:")) {
                const uid_t uid = line.section('\t', 1, 1).toUInt();
                if (const passwd *pw = getpwuid(uid))
                    p.user = QString::fromLocal8Bit(pw->pw_name);
            } else if (line.startsWith("VmRSS:")) {
                p.rssKb = line.section(QRegularExpression("\\s+"), 1, 1).toLongLong();
            }
        }
        result.append(p);
    }
    return result;
}