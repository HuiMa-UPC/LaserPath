#include "DatabaseMigrator.h"
#include "IDatabaseProvider.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>

static const QList<Migration> kMigrations = {
    { 1, "Create process_profiles table",
      "CREATE TABLE IF NOT EXISTS process_profiles ("
      "  id                    INTEGER PRIMARY KEY AUTOINCREMENT,"
      "  name                  TEXT    NOT NULL UNIQUE,"

    /************************** 切片参数 **************************/
      "  layer_height          REAL    DEFAULT 2.0,"
      "  first_layer_height    REAL    DEFAULT 2.0,"
      "  auto_range            INTEGER DEFAULT 1,"
      "  z_min                 REAL    DEFAULT 0.0,"
      "  z_max                 REAL    DEFAULT 0.0,"

    /************************** 精度参数 **************************/
      "  min_contour_len       REAL    DEFAULT 2.0,"
      "  collinear_precision   REAL    DEFAULT 2.0,"
      "  gap_close_radius      REAL    DEFAULT 2.0,"

    /************************** 偏移参数 **************************/
      "  enable_offset         INTEGER DEFAULT 1,"
      "  outer_wall_width      REAL    DEFAULT 8.0,"
      "  inner_wall_width      REAL    DEFAULT 8.0,"
      "  base_contour          INTEGER DEFAULT 0,"
      "  auto_offset           INTEGER DEFAULT 0,"
      "  offset_count          INTEGER DEFAULT 1,"
      "  offset_contour_comp   REAL    DEFAULT 0.0,"
      "  start_angle           REAL    DEFAULT 0.0,"
      "  start_point_strategy  INTEGER DEFAULT 0,"
      "  angle_increment       REAL    DEFAULT 0.0,"
      "  length_increment      REAL    DEFAULT 0.0,"

    /************************** 填充参数 **************************/
      "  bead_width            REAL    DEFAULT 8.0,"
      "  fill_type             INTEGER DEFAULT 0,"
      "  adaptive_overlap      INTEGER DEFAULT 0,"
      "  max_overlap_rate      REAL    DEFAULT 0.0,"
      "  min_overlap_rate      REAL    DEFAULT 0.0,"
      "  overlap_rate          REAL    DEFAULT 0.0,"
      "  margin_strategy       INTEGER DEFAULT 0,"
      "  scan_direction        INTEGER DEFAULT 0,"
      "  scan_angle            REAL    DEFAULT 0.0,"
      "  interlayer_rotation   REAL    DEFAULT 0.0,"
      "  fill_area_comp        REAL    DEFAULT 0.0,"
      "  end_extension         REAL    DEFAULT 0.0,"
      "  created_at            TEXT,"
      "  updated_at            TEXT"
      ");"
      "INSERT INTO process_profiles (name)"
      "  SELECT '默认工艺'"
      "  WHERE NOT EXISTS (SELECT 1 FROM process_profiles)" },

    { 2, "Add created_at and updated_at columns to process_profiles",
      "ALTER TABLE process_profiles ADD COLUMN created_at TEXT;"
      "ALTER TABLE process_profiles ADD COLUMN updated_at TEXT" },

};

/************************** 构造函数 **************************/
DatabaseMigrator::DatabaseMigrator(IDatabaseProvider& provider)
    : m_provider(provider)
{
}

/************************** 版本迁移 **************************/
bool DatabaseMigrator::migrate()
{
    // 1. 确保 VersionTable 存在
    if (!ensureVersionTable())
        return false;

    // 2. 若 currentVersion 等于0，说明版本记录表尚未初始化；
    // 需要探测当前数据库实际版本，并将版本信息写入VersionTable。
    int current = currentVersion();
    if (current == 0) {
        current = detectBootstrapVersion();
        if (current > 0) {
            qInfo() << "DatabaseMigrator: bootstrapped existing schema as version"
                << current;
            // Record synthetic version entries so we don't re-apply
            for (int v = 1; v <= current; ++v)
                recordVersion(v);
        }
    }

    // 3. 根据 VersionTable 中记录的最高版本号，
    // 筛选 kMigrations 中版本号高于该值、尚未执行的迁移项，将对应字段追加到 process_profiles 表。
    QList<Migration> pending = pendingMigrations();
    for (const Migration& m : pending) {
        if (!applyMigration(m))
            return false;
        recordVersion(m.version);
        qInfo() << "DatabaseMigrator: applied v" << m.version
            << "-" << m.description;
    }

    return true;
}

/************************** 目前版本号 **************************/
int DatabaseMigrator::currentVersion() const
{
    QSqlQuery q(m_provider.db());
    q.exec("SELECT COALESCE(MAX(version), 0) FROM schema_version");
    if (q.next())
        return q.value(0).toInt();
    return 0;
}

/************************** 确保 VersionTable 存在 **************************/
bool DatabaseMigrator::ensureVersionTable()
{
    QSqlQuery q(m_provider.db());
    const QString sql =
        "CREATE TABLE IF NOT EXISTS schema_version ("
        "  version    INTEGER PRIMARY KEY,"
        "  applied_at TEXT    NOT NULL"
        ")";
    if (!q.exec(sql)) {
        qWarning() << "DatabaseMigrator: cannot create schema_version table"
            << q.lastError().text();
        return false;
    }
    return true;
}

/************************** 需要迁移的字段 **************************/
QList<Migration> DatabaseMigrator::pendingMigrations() const
{
    int current = currentVersion();
    QList<Migration> pending;
    for (const Migration& m : kMigrations) {
        if (m.version > current)
            pending.append(m);
    }
    return pending;
}

/************************** 根据数据库现有的字段识别版本号 **************************/
int DatabaseMigrator::detectBootstrapVersion() const
{
    QSqlDatabase& db = m_provider.db();

    // Does the table exist at all?
    if (!db.tables().contains("machine_profiles"))
        return 0;  // fresh database — apply all migrations

    QSqlRecord rec = db.record("machine_profiles");

    // The last migration added max_accel_* columns — if present, v4+
    if (rec.contains("max_accel_e"))
        return 4;

    // If speed columns exist but accel doesn't, we're at v3
    if (rec.contains("max_speed_e"))
        return 3;

    // If speed x exists but not the others, we're at v2
    if (rec.contains("max_speed_x"))
        return 2;

    // Table exists with core columns — at least v1
    if (rec.contains("name"))
        return 1;

    return 0;
}

/************************** 迁移的待执行的字段 **************************/
bool DatabaseMigrator::applyMigration(const Migration& m)
{
    QSqlQuery q(m_provider.db());

    // A migration may contain multiple semicolon-separated statements.
    // SQLite's QSqlQuery::exec supports only one statement at a time
    // via the Qt SQL driver, so we split and execute individually.
    const QStringList statements = m.sql.split(';', Qt::SkipEmptyParts);
    for (const QString& stmt : statements) {
        const QString trimmed = stmt.trimmed();
        if (trimmed.isEmpty())
            continue;

        if (!q.exec(trimmed)) {
            QString err = q.lastError().text();
            // 幂等处理: 列/表已存在时跳过，不视为错误
            if (err.contains(QStringLiteral("duplicate column"), Qt::CaseInsensitive) ||
                err.contains(QStringLiteral("already exists"), Qt::CaseInsensitive)) {
                qInfo() << "DatabaseMigrator: skipping (already exists) —" << trimmed;
                continue;
            }
            qWarning() << "DatabaseMigrator: migration v" << m.version
                << "failed —" << err
                << "\n  SQL:" << trimmed;
            return false;
        }
    }
    return true;
}

/************************** 记录版本 **************************/
void DatabaseMigrator::recordVersion(int version)
{
    QSqlQuery q(m_provider.db());
    q.prepare("INSERT OR REPLACE INTO schema_version (version, applied_at) "
        "VALUES (?, datetime('now'))");
    q.addBindValue(version);
    q.exec();  // failure here is non-fatal — will retry on next migrate()
}
