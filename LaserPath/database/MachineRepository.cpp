#include "MachineRepository.h"
#include "IDatabaseProvider.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QDebug>

static const char* kMachineTable = "machine_profiles";

/************************** 构造函数 **************************/
MachineRepository::MachineRepository(IDatabaseProvider& provider)
    : m_provider(provider)
{
}

QList<QPair<int, QString>> MachineRepository::listAllName() const
{
    QList<QPair<int, QString>> result;
    QSqlQuery q(m_provider.db());
    q.exec(QString("SELECT id, name FROM %1 ORDER BY id").arg(kMachineTable));
    while (q.next())
        result.append({q.value(0).toInt(), q.value(1).toString()});
    return result;
}

/************************ Row mapping（读取） ************************/
MachineProfile MachineRepository::profileFromQuery(const QSqlQuery& q)
{
    MachineProfile p;
    p.id                = q.value("id").toInt();
    p.name              = q.value("name").toString();
    p.createdAt         = q.value("created_at").toString();
    p.updatedAt         = q.value("updated_at").toString();

    // 基础信息
    p.kinematics        = q.value("kinematics").toInt();
    p.heatSource        = q.value("heat_source").toInt();
    p.maxHeight         = q.value("max_height").toDouble();
    p.maxX              = q.value("max_x").toDouble();
    p.maxY              = q.value("max_y").toDouble();
    p.originX           = q.value("origin_x").toDouble();
    p.originY           = q.value("origin_y").toDouble();
    p.imagePath         = q.value("image_path").toString();
    p.imageData         = q.value("image_data").toByteArray();

    // G-code
    p.printStartGcode   = q.value("print_start_gcode").toString();
    p.printEndGcode     = q.value("print_end_gcode").toString();
    p.pathStartGcode    = q.value("path_start_gcode").toString();
    p.pathEndGcode      = q.value("path_end_gcode").toString();
    p.layerStartGcode   = q.value("layer_start_gcode").toString();
    p.layerEndGcode     = q.value("layer_end_gcode").toString();
    p.interlayerCoolingGcode = q.value("interlayer_cooling_gcode").toString();

    // 运动能力
    p.maxSpeedX         = q.value("max_speed_x").toDouble();
    p.maxSpeedY         = q.value("max_speed_y").toDouble();
    p.maxSpeedZ         = q.value("max_speed_z").toDouble();
    p.maxSpeedE         = q.value("max_speed_e").toDouble();

    // 备注
    p.notes             = q.value("notes").toString();

    return p;
}

/************************** 写入 binding（INSERT） **************************/
static void bindInsertValues(QSqlQuery& q, const MachineProfile& p)
{
    q.bindValue(0,  p.name);
    q.bindValue(1,  p.kinematics);
    q.bindValue(2,  p.heatSource);
    q.bindValue(3,  p.maxHeight);
    q.bindValue(4,  p.maxX);
    q.bindValue(5,  p.maxY);
    q.bindValue(6,  p.originX);
    q.bindValue(7,  p.originY);
    q.bindValue(8,  p.imagePath);
    q.bindValue(9,  p.imageData);
    q.bindValue(10, p.printStartGcode);
    q.bindValue(11, p.printEndGcode);
    q.bindValue(12, p.pathStartGcode);
    q.bindValue(13, p.pathEndGcode);
    q.bindValue(14, p.layerStartGcode);
    q.bindValue(15, p.layerEndGcode);
    q.bindValue(16, p.interlayerCoolingGcode);
    q.bindValue(17, p.maxSpeedX);
    q.bindValue(18, p.maxSpeedY);
    q.bindValue(19, p.maxSpeedZ);
    q.bindValue(20, p.maxSpeedE);
    q.bindValue(21, p.notes);
    q.bindValue(22, p.createdAt);
    q.bindValue(23, p.updatedAt);
}

/************************** 写入 binding（UPDATE） **************************/
static void bindUpdateValues(QSqlQuery& q, const MachineProfile& p)
{
    q.bindValue(0,  p.name);
    q.bindValue(1,  p.kinematics);
    q.bindValue(2,  p.heatSource);
    q.bindValue(3,  p.maxHeight);
    q.bindValue(4,  p.maxX);
    q.bindValue(5,  p.maxY);
    q.bindValue(6,  p.originX);
    q.bindValue(7,  p.originY);
    q.bindValue(8,  p.imagePath);
    q.bindValue(9,  p.imageData);
    q.bindValue(10, p.printStartGcode);
    q.bindValue(11, p.printEndGcode);
    q.bindValue(12, p.pathStartGcode);
    q.bindValue(13, p.pathEndGcode);
    q.bindValue(14, p.layerStartGcode);
    q.bindValue(15, p.layerEndGcode);
    q.bindValue(16, p.interlayerCoolingGcode);
    q.bindValue(17, p.maxSpeedX);
    q.bindValue(18, p.maxSpeedY);
    q.bindValue(19, p.maxSpeedZ);
    q.bindValue(20, p.maxSpeedE);
    q.bindValue(21, p.notes);
    q.bindValue(22, p.updatedAt);
    q.bindValue(23, p.id);
}

/************************** 获取全部 **************************/
QList<MachineProfile> MachineRepository::listAll() const
{
    QList<MachineProfile> results;
    QSqlQuery q(m_provider.db());
    q.exec(QString("SELECT * FROM %1 ORDER BY id").arg(kMachineTable));
    while (q.next())
        results.append(profileFromQuery(q));
    return results;
}

/************************** 根据 Id 查询 **************************/
MachineProfile MachineRepository::getById(int id) const
{
    QSqlQuery q(m_provider.db());
    q.prepare(QString("SELECT * FROM %1 WHERE id = ?").arg(kMachineTable));
    q.addBindValue(id);
    if (q.exec() && q.next())
        return profileFromQuery(q);
    return MachineProfile();
}

/************************** 保存 **************************/
bool MachineRepository::save(MachineProfile& profile)
{
    QSqlQuery q(m_provider.db());
    QString now = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    bool prepared = false;
    if (profile.id == 0) {
        // INSERT — 24 列
        profile.createdAt = now;
        prepared = q.prepare(QString(
            "INSERT INTO %1 (name,"
            " kinematics, heat_source,"
            " max_height, max_x, max_y, origin_x, origin_y, image_path, image_data,"
            " print_start_gcode, print_end_gcode,"
            " path_start_gcode, path_end_gcode,"
            " layer_start_gcode, layer_end_gcode,"
            " interlayer_cooling_gcode,"
            " max_speed_x, max_speed_y, max_speed_z, max_speed_e,"
            " notes, created_at, updated_at)"
            " VALUES ("
            " ?,?,?,?,?,?,?,?,?,?,"
            " ?,?,?,?,?,?,?,?,?,?,"
            " ?,?,?,?"
            ")"
        ).arg(kMachineTable));
    }
    else {
        // UPDATE — 23 字段 + id
        prepared = q.prepare(QString(
            "UPDATE %1 SET"
            " name=?, kinematics=?, heat_source=?,"
            " max_height=?, max_x=?, max_y=?,"
            " origin_x=?, origin_y=?, image_path=?, image_data=?,"
            " print_start_gcode=?, print_end_gcode=?,"
            " path_start_gcode=?, path_end_gcode=?,"
            " layer_start_gcode=?, layer_end_gcode=?,"
            " interlayer_cooling_gcode=?,"
            " max_speed_x=?, max_speed_y=?, max_speed_z=?, max_speed_e=?,"
            " notes=?,"
            " updated_at=?"
            " WHERE id=?"
        ).arg(kMachineTable));
    }

    if (!prepared) {
        qWarning() << "MachineRepository::save prepare failed:" << q.lastError().text();
        return false;
    }

    profile.updatedAt = now;

    if (profile.id == 0)
        bindInsertValues(q, profile);
    else
        bindUpdateValues(q, profile);

    qDebug() << "=== MachineRepository::save ===";
    qDebug() << "  Mode:" << (profile.id == 0 ? "INSERT" : "UPDATE");
    qDebug() << "  SQL:" << q.lastQuery();
    qDebug() << "  Bound values count:" << q.boundValues().size();

    if (!q.exec()) {
        qWarning() << "MachineRepository::save exec failed:" << q.lastError().text();
        return false;
    }

    if (profile.id == 0)
        profile.id = q.lastInsertId().toInt();
    return true;
}

/************************** 复制 **************************/
MachineProfile MachineRepository::copy(int id)
{
    MachineProfile source = getById(id);
    if (source.id == 0)
        return MachineProfile();

    MachineProfile dup = source;
    dup.id = 0;
    dup.name = source.name + QStringLiteral(" - 副本");
    dup.createdAt.clear();
    dup.updatedAt.clear();

    if (!save(dup))
        return MachineProfile();
    return dup;
}

/************************** 删除 **************************/
bool MachineRepository::remove(int id)
{
    if (id == 0) return false;
    QSqlQuery q(m_provider.db());
    q.prepare(QString("DELETE FROM %1 WHERE id = ?").arg(kMachineTable));
    q.addBindValue(id);
    if (!q.exec()) {
        qWarning() << "MachineRepository::remove failed:" << q.lastError().text();
        return false;
    }
    return true;
}
