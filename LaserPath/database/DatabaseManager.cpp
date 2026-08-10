#include "DatabaseManager.h"
#include "DatabaseMigrator.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager inst;
    return inst;
}

DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen())
        m_db.close();
}

bool DatabaseManager::init(const QString& dbPath)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        qWarning() << "DatabaseManager: failed to open" << dbPath
            << m_db.lastError().text();
        return false;
    }

    QSqlQuery pragma(m_db);
    pragma.exec("PRAGMA journal_mode=WAL");

    return createTables();
}

QSqlDatabase& DatabaseManager::db()
{
    return m_db;
}

bool DatabaseManager::createTables()
{
    DatabaseMigrator migrator(*this);
    return migrator.migrate();
}
