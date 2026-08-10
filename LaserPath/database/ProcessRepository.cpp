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

/************************** 写入 binding **************************/
static void bindProfileValues(QSqlQuery& q, const ProcessProfile& p)
{
    q.bindValue(":name",                p.name);

    // 切片
    q.bindValue(":layer_height",        p.layerHeight);
    q.bindValue(":first_layer_height",  p.firstLayerHeight);
    q.bindValue(":auto_range",          p.autoRange);
    q.bindValue(":z_min",               p.zMin);
    q.bindValue(":z_max",               p.zMax);

    // 精度
    q.bindValue(":min_contour_len",     p.minContourLen);
    q.bindValue(":collinear_precision", p.collinearPrecision);
    q.bindValue(":gap_close_radius",    p.gapCloseRadius);

    // 偏移
    q.bindValue(":enable_offset",       p.enableOffset);
    q.bindValue(":outer_wall_width",    p.outerWallWidth);
    q.bindValue(":inner_wall_width",    p.innerWallWidth);
    q.bindValue(":base_contour",        p.baseContour);
    q.bindValue(":auto_offset",         p.autoOffset);
    q.bindValue(":offset_count",        p.offsetCount);
    q.bindValue(":offset_contour_comp", p.offsetContourComp);
    q.bindValue(":start_angle",         p.startAngle);
    q.bindValue(":start_point_strategy", p.startPointStrategy);
    q.bindValue(":angle_increment",     p.angleIncrement);
    q.bindValue(":length_increment",    p.lengthIncrement);

    // 填充
    q.bindValue(":bead_width",          p.beadWidth);
    q.bindValue(":fill_type",           p.fillType);
    q.bindValue(":adaptive_overlap",    p.adaptiveOverlap);
    q.bindValue(":max_overlap_rate",    p.maxOverlapRate);
    q.bindValue(":min_overlap_rate",    p.minOverlapRate);
    q.bindValue(":overlap_rate",        p.overlapRate);
    q.bindValue(":margin_strategy",     p.marginStrategy);
    q.bindValue(":scan_direction",      p.scanDirection);
    q.bindValue(":scan_angle",          p.scanAngle);
    q.bindValue(":interlayer_rotation", p.interlayerRotation);
    q.bindValue(":fill_area_comp",      p.fillAreaComp);
    q.bindValue(":end_extension",       p.endExtension);

    q.bindValue(":created_at",          p.createdAt);
    q.bindValue(":updated_at",          p.updatedAt);
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

    if (profile.id == 0) {
        // INSERT
        profile.createdAt = now;
        q.prepare(QString(
            "INSERT INTO %1 ("
            "  name,"
            "  layer_height, first_layer_height, auto_range, z_min, z_max,"
            "  min_contour_len, collinear_precision, gap_close_radius,"
            "  enable_offset, outer_wall_width, inner_wall_width, base_contour,"
            "  auto_offset, offset_count, offset_contour_comp, start_angle,"
            "  start_point_strategy, angle_increment, length_increment,"
            "  bead_width, fill_type, adaptive_overlap,"
            "  max_overlap_rate, min_overlap_rate, overlap_rate,"
            "  margin_strategy, scan_direction, scan_angle,"
            "  interlayer_rotation, fill_area_comp, end_extension,"
            "  created_at, updated_at"
            ") VALUES ("
            "  :name,"
            "  :layer_height, :first_layer_height, :auto_range, :z_min, :z_max,"
            "  :min_contour_len, :collinear_precision, :gap_close_radius,"
            "  :enable_offset, :outer_wall_width, :inner_wall_width, :base_contour,"
            "  :auto_offset, :offset_count, :offset_contour_comp, :start_angle,"
            "  :start_point_strategy, :angle_increment, :length_increment,"
            "  :bead_width, :fill_type, :adaptive_overlap,"
            "  :max_overlap_rate, :min_overlap_rate, :overlap_rate,"
            "  :margin_strategy, :scan_direction, :scan_angle,"
            "  :interlayer_rotation, :fill_area_comp, :end_extension,"
            "  :created_at, :updated_at"
            ")"
        ).arg(kTable));
    }
    else {
        // UPDATE
        q.prepare(QString(
            "UPDATE %1 SET "
            "  name=:name,"
            "  layer_height=:layer_height, first_layer_height=:first_layer_height,"
            "  auto_range=:auto_range, z_min=:z_min, z_max=:z_max,"
            "  min_contour_len=:min_contour_len, collinear_precision=:collinear_precision,"
            "  gap_close_radius=:gap_close_radius,"
            "  enable_offset=:enable_offset, outer_wall_width=:outer_wall_width,"
            "  inner_wall_width=:inner_wall_width, base_contour=:base_contour,"
            "  auto_offset=:auto_offset, offset_count=:offset_count,"
            "  offset_contour_comp=:offset_contour_comp, start_angle=:start_angle,"
            "  start_point_strategy=:start_point_strategy, angle_increment=:angle_increment,"
            "  length_increment=:length_increment,"
            "  bead_width=:bead_width, fill_type=:fill_type,"
            "  adaptive_overlap=:adaptive_overlap, max_overlap_rate=:max_overlap_rate,"
            "  min_overlap_rate=:min_overlap_rate, overlap_rate=:overlap_rate,"
            "  margin_strategy=:margin_strategy, scan_direction=:scan_direction,"
            "  scan_angle=:scan_angle, interlayer_rotation=:interlayer_rotation,"
            "  fill_area_comp=:fill_area_comp, end_extension=:end_extension,"
            "  updated_at=:updated_at"
            " WHERE id=:id"
        ).arg(kTable));
        q.bindValue(":id", profile.id);
    }
    profile.updatedAt = now;

    bindProfileValues(q, profile);

    if (!q.exec()) {
        qWarning() << "ProcessRepository::save failed:" << q.lastError().text();
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
