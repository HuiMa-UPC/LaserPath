#pragma once

#include <QSqlDatabase>
#include <QString>
#include "IDatabaseProvider.h"

class DatabaseManager : public IDatabaseProvider {
public:
    static DatabaseManager& instance();

    bool init(const QString& dbPath);
    QSqlDatabase& db() override;

private:
    DatabaseManager() = default;
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    bool createTables();

    QSqlDatabase m_db;
};
