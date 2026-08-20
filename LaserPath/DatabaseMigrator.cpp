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

    { 3, "Create machine_profiles table",
      "CREATE TABLE IF NOT EXISTS machine_profiles ("
      "  id                  INTEGER PRIMARY KEY AUTOINCREMENT,"
      "  name                TEXT    NOT NULL UNIQUE,"
      "  kinematics          INTEGER DEFAULT 0,"
      "  heat_source         INTEGER DEFAULT 0,"
      "  max_height          REAL    DEFAULT 500.0,"
      "  max_x               REAL    DEFAULT 500.0,"
      "  max_y               REAL    DEFAULT 500.0,"
      "  origin_x            REAL    DEFAULT 0.0,"
      "  origin_y            REAL    DEFAULT 0.0,"
      "  image_path          TEXT    DEFAULT '',"
      "  image_data          BLOB,"
      "  print_start_gcode   TEXT    DEFAULT '',"
      "  print_end_gcode     TEXT    DEFAULT '',"
      "  path_start_gcode    TEXT    DEFAULT '',"
      "  path_end_gcode      TEXT    DEFAULT '',"
      "  layer_start_gcode   TEXT    DEFAULT '',"
      "  layer_end_gcode     TEXT    DEFAULT '',"
      "  interlayer_cooling_gcode TEXT DEFAULT '',"
      "  max_speed_x         REAL    DEFAULT 1000.0,"
      "  max_speed_y         REAL    DEFAULT 1000.0,"
      "  max_speed_z         REAL    DEFAULT 1000.0,"
      "  max_speed_e         REAL    DEFAULT 1000.0,"
      "  notes               TEXT    DEFAULT '',"
      "  created_at          TEXT,"
      "  updated_at          TEXT"
      ");"
      "INSERT INTO machine_profiles (name)"
      "  SELECT 'Rongsu Laser One'"
      "  WHERE NOT EXISTS (SELECT 1 FROM machine_profiles)" },

    { 4, "Add image_data column to machine_profiles",
      "ALTER TABLE machine_profiles ADD COLUMN image_data BLOB" },

    { 5, "Add G-code columns and image_path to machine_profiles",
      "ALTER TABLE machine_profiles ADD COLUMN image_path TEXT DEFAULT '';"
      "ALTER TABLE machine_profiles ADD COLUMN print_start_gcode TEXT DEFAULT '';"
      "ALTER TABLE machine_profiles ADD COLUMN print_end_gcode TEXT DEFAULT '';"
      "ALTER TABLE machine_profiles ADD COLUMN path_start_gcode TEXT DEFAULT '';"
      "ALTER TABLE machine_profiles ADD COLUMN path_end_gcode TEXT DEFAULT '';"
      "ALTER TABLE machine_profiles ADD COLUMN layer_start_gcode TEXT DEFAULT '';"
      "ALTER TABLE machine_profiles ADD COLUMN layer_end_gcode TEXT DEFAULT ''" },

    { 6, "Add interlayer cooling G-code column to machine_profiles",
      "ALTER TABLE machine_profiles ADD COLUMN interlayer_cooling_gcode TEXT DEFAULT ''" },

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

    // Check machine_profiles first
    if (db.tables().contains("machine_profiles")) {
        QSqlRecord rec = db.record("machine_profiles");

        // v6+ has the interlayer cooling G-code column
        if (rec.contains("interlayer_cooling_gcode"))
            return 6;

        // v5+ has the six G-code columns
        if (rec.contains("layer_end_gcode"))
            return 5;

        // v4+ has the image_data column
        if (rec.contains("image_data"))
            return 4;

        // v3+ has the notes column
        if (rec.contains("notes"))
            return 3;

        // Legacy detection for older machine_profiles schemas
        if (rec.contains("max_speed_e"))
            return 3;

        if (rec.contains("max_speed_x"))
            return 2;

        if (rec.contains("name"))
            return 1;
    }

    // If process_profiles exists, we're at least at v2
    if (db.tables().contains("process_profiles"))
        return 2;

    return 0;  // fresh database — apply all migrations
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
