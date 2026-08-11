#include "ProcessRepository.h"
#include "IDatabaseProvider.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QDebug>

static const char* kTable = "process_profiles";

/************************** 构造函数 **************************/
ProcessRepository::ProcessRepository(IDatabaseProvider& provider)
    : m_provider(provider)
{
}

/************************ Row mapping（读取） ************************/
ProcessProfile ProcessRepository::profileFromQuery(const QSqlQuery& q)
{
    ProcessProfile p;
    p.id                  = q.value("id").toInt();
    p.name                = q.value("name").toString();
    p.createdAt           = q.value("created_at").toString();
    p.updatedAt           = q.value("updated_at").toString();

    // 切片
    p.layerHeight         = q.value("layer_height").toDouble();
    p.firstLayerHeight    = q.value("first_layer_height").toDouble();
    p.autoRange           = q.value("auto_range").toBool();
    p.zMin                = q.value("z_min").toDouble();
    p.zMax                = q.value("z_max").toDouble();

    // 精度
    p.minContourLen       = q.value("min_contour_len").toDouble();
    p.collinearPrecision  = q.value("collinear_precision").toDouble();
    p.gapCloseRadius      = q.value("gap_close_radius").toDouble();

    // 偏移
    p.enableOffset        = q.value("enable_offset").toBool();
    p.outerWallWidth      = q.value("outer_wall_width").toDouble();
    p.innerWallWidth      = q.value("inner_wall_width").toDouble();
    p.baseContour         = q.value("base_contour").toInt();
    p.autoOffset          = q.value("auto_offset").toBool();
    p.offsetCount         = q.value("offset_count").toInt();
    p.offsetContourComp   = q.value("offset_contour_comp").toDouble();
    p.startAngle          = q.value("start_angle").toDouble();
    p.startPointStrategy  = q.value("start_point_strategy").toInt();
    p.angleIncrement      = q.value("angle_increment").toDouble();
    p.lengthIncrement     = q.value("length_increment").toDouble();

    // 填充
    p.beadWidth           = q.value("bead_width").toDouble();
    p.fillType            = q.value("fill_type").toInt();
    p.adaptiveOverlap     = q.value("adaptive_overlap").toBool();
    p.maxOverlapRate      = q.value("max_overlap_rate").toDouble();
    p.minOverlapRate      = q.value("min_overlap_rate").toDouble();
    p.overlapRate         = q.value("overlap_rate").toDouble();
    p.marginStrategy      = q.value("margin_strategy").toInt();
    p.scanDirection       = q.value("scan_direction").toInt();
    p.scanAngle           = q.value("scan_angle").toDouble();
    p.interlayerRotation  = q.value("interlayer_rotation").toDouble();
    p.fillAreaComp        = q.value("fill_area_comp").toDouble();
    p.endExtension        = q.value("end_extension").toDouble();

    return p;
}

/************************** 写入 binding（INSERT） **************************/
static void bindInsertValues(QSqlQuery& q, const ProcessProfile& p)
{
    q.bindValue(0,  p.name);
    // 切片
    q.bindValue(1,  p.layerHeight);
    q.bindValue(2,  p.firstLayerHeight);
    q.bindValue(3,  p.autoRange);
    q.bindValue(4,  p.zMin);
    q.bindValue(5,  p.zMax);
    // 精度
    q.bindValue(6,  p.minContourLen);
    q.bindValue(7,  p.collinearPrecision);
    q.bindValue(8,  p.gapCloseRadius);
    // 偏移
    q.bindValue(9,  p.enableOffset);
    q.bindValue(10, p.outerWallWidth);
    q.bindValue(11, p.innerWallWidth);
    q.bindValue(12, p.baseContour);
    q.bindValue(13, p.autoOffset);
    q.bindValue(14, p.offsetCount);
    q.bindValue(15, p.offsetContourComp);
    q.bindValue(16, p.startAngle);
    q.bindValue(17, p.startPointStrategy);
    q.bindValue(18, p.angleIncrement);
    q.bindValue(19, p.lengthIncrement);
    // 填充
    q.bindValue(20, p.beadWidth);
    q.bindValue(21, p.fillType);
    q.bindValue(22, p.adaptiveOverlap);
    q.bindValue(23, p.maxOverlapRate);
    q.bindValue(24, p.minOverlapRate);
    q.bindValue(25, p.overlapRate);
    q.bindValue(26, p.marginStrategy);
    q.bindValue(27, p.scanDirection);
    q.bindValue(28, p.scanAngle);
    q.bindValue(29, p.interlayerRotation);
    q.bindValue(30, p.fillAreaComp);
    q.bindValue(31, p.endExtension);
    q.bindValue(32, p.createdAt);
    q.bindValue(33, p.updatedAt);
}

/************************** 写入 binding（UPDATE） **************************/
static void bindUpdateValues(QSqlQuery& q, const ProcessProfile& p)
{
    q.bindValue(0,  p.name);
    q.bindValue(1,  p.layerHeight);
    q.bindValue(2,  p.firstLayerHeight);
    q.bindValue(3,  p.autoRange);
    q.bindValue(4,  p.zMin);
    q.bindValue(5,  p.zMax);
    q.bindValue(6,  p.minContourLen);
    q.bindValue(7,  p.collinearPrecision);
    q.bindValue(8,  p.gapCloseRadius);
    q.bindValue(9,  p.enableOffset);
    q.bindValue(10, p.outerWallWidth);
    q.bindValue(11, p.innerWallWidth);
    q.bindValue(12, p.baseContour);
    q.bindValue(13, p.autoOffset);
    q.bindValue(14, p.offsetCount);
    q.bindValue(15, p.offsetContourComp);
    q.bindValue(16, p.startAngle);
    q.bindValue(17, p.startPointStrategy);
    q.bindValue(18, p.angleIncrement);
    q.bindValue(19, p.lengthIncrement);
    q.bindValue(20, p.beadWidth);
    q.bindValue(21, p.fillType);
    q.bindValue(22, p.adaptiveOverlap);
    q.bindValue(23, p.maxOverlapRate);
    q.bindValue(24, p.minOverlapRate);
    q.bindValue(25, p.overlapRate);
    q.bindValue(26, p.marginStrategy);
    q.bindValue(27, p.scanDirection);
    q.bindValue(28, p.scanAngle);
    q.bindValue(29, p.interlayerRotation);
    q.bindValue(30, p.fillAreaComp);
    q.bindValue(31, p.endExtension);
    q.bindValue(32, p.updatedAt);
    q.bindValue(33, p.id);
}

/************************** 获取全部 **************************/
QList<ProcessProfile> ProcessRepository::listAll() const
{
    QList<ProcessProfile> results;
    QSqlQuery q(m_provider.db());
    q.exec(QString("SELECT * FROM %1 ORDER BY id").arg(kTable));
    while (q.next())
        results.append(profileFromQuery(q));
    return results;
}

/************************** 根据 Id 查询 **************************/
ProcessProfile ProcessRepository::getById(int id) const
{
    QSqlQuery q(m_provider.db());
    q.prepare(QString("SELECT * FROM %1 WHERE id = ?").arg(kTable));
    q.addBindValue(id);
    if (q.exec() && q.next())
        return profileFromQuery(q);
    return ProcessProfile();
}

/************************** 保存 **************************/
bool ProcessRepository::save(ProcessProfile& profile)
{
    QSqlQuery q(m_provider.db());
    QString now = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    bool prepared = false;
    if (profile.id == 0) {
        // INSERT — 34 个 ?
        profile.createdAt = now;
        prepared = q.prepare(QString(
            "INSERT INTO %1 (name,"
            " layer_height, first_layer_height, auto_range, z_min, z_max,"
            " min_contour_len, collinear_precision, gap_close_radius,"
            " enable_offset, outer_wall_width, inner_wall_width, base_contour,"
            " auto_offset, offset_count, offset_contour_comp, start_angle,"
            " start_point_strategy, angle_increment, length_increment,"
            " bead_width, fill_type, adaptive_overlap,"
            " max_overlap_rate, min_overlap_rate, overlap_rate,"
            " margin_strategy, scan_direction, scan_angle,"
            " interlayer_rotation, fill_area_comp, end_extension,"
            " created_at, updated_at)"
            " VALUES ("
            " ?,?,?,?,?,?,?,?,?,?,"
            " ?,?,?,?,?,?,?,?,?,?,"
            " ?,?,?,?,?,?,?,?,?,?,"
            " ?,?,?,?"
            ")"
        ).arg(kTable));
    }
    else {
        // UPDATE — 33 个字段 + id
        prepared = q.prepare(QString(
            "UPDATE %1 SET"
            " name=?, layer_height=?, first_layer_height=?,"
            " auto_range=?, z_min=?, z_max=?,"
            " min_contour_len=?, collinear_precision=?, gap_close_radius=?,"
            " enable_offset=?, outer_wall_width=?, inner_wall_width=?,"
            " base_contour=?, auto_offset=?, offset_count=?,"
            " offset_contour_comp=?, start_angle=?,"
            " start_point_strategy=?, angle_increment=?, length_increment=?,"
            " bead_width=?, fill_type=?, adaptive_overlap=?,"
            " max_overlap_rate=?, min_overlap_rate=?, overlap_rate=?,"
            " margin_strategy=?, scan_direction=?, scan_angle=?,"
            " interlayer_rotation=?, fill_area_comp=?, end_extension=?,"
            " updated_at=?"
            " WHERE id=?"
        ).arg(kTable));
    }

    if (!prepared) {
        qWarning() << "ProcessRepository::save prepare failed:" << q.lastError().text();
        return false;
    }

    profile.updatedAt = now;

    if (profile.id == 0)
        bindInsertValues(q, profile);
    else
        bindUpdateValues(q, profile);

    // DEBUG: 打印 SQL 和绑定参数数量
    qDebug() << "=== ProcessRepository::save ===";
    qDebug() << "  Mode:" << (profile.id == 0 ? "INSERT" : "UPDATE");
    qDebug() << "  SQL:" << q.lastQuery();
    qDebug() << "  Bound values count:" << q.boundValues().size();

    if (!q.exec()) {
        qWarning() << "ProcessRepository::save exec failed:" << q.lastError().text();
        return false;
    }

    if (profile.id == 0)
        profile.id = q.lastInsertId().toInt();
    return true;
}

/************************** 复制 **************************/
ProcessProfile ProcessRepository::copy(int id)
{
    ProcessProfile source = getById(id);
    if (source.id == 0)
        return ProcessProfile();

    ProcessProfile dup = source;
    dup.id = 0;
    dup.name = source.name + QStringLiteral(" - 副本");
    dup.createdAt.clear();
    dup.updatedAt.clear();

    if (!save(dup))
        return ProcessProfile();
    return dup;
}

/************************** 删除 **************************/
bool ProcessRepository::remove(int id)
{
    if (id == 0) return false;
    QSqlQuery q(m_provider.db());
    q.prepare(QString("DELETE FROM %1 WHERE id = ?").arg(kTable));
    q.addBindValue(id);
    if (!q.exec()) {
        qWarning() << "ProcessRepository::remove failed:" << q.lastError().text();
        return false;
    }
    return true;
}
