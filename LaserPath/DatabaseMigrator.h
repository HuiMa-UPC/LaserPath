#pragma once

#include <QString>
#include <QList>

class IDatabaseProvider;

struct Migration {
    int         version;
    QString     description;
    QString     sql;
};

class DatabaseMigrator {
public:
    explicit DatabaseMigrator(IDatabaseProvider& provider);

    bool migrate();

    int currentVersion() const;

private:
    bool ensureVersionTable();
    QList<Migration> pendingMigrations() const;
    int detectBootstrapVersion() const;
    bool applyMigration(const Migration& m);
    void recordVersion(int version);

    IDatabaseProvider& m_provider;
};
