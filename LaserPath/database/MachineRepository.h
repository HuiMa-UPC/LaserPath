#pragma once

#include <QList>
#include <QPair>
#include "MachineProfile.h"

class IDatabaseProvider;
class QSqlQuery;

class MachineRepository {
public:
    explicit MachineRepository(IDatabaseProvider& provider);
    QList<QPair<int, QString>> listAllName() const;
    QList<MachineProfile>      listAll() const;
    MachineProfile             getById(int id) const;
    bool                       save(MachineProfile& profile);
    MachineProfile             copy(int id);
    bool                       remove(int id);

private:
    IDatabaseProvider& m_provider;

    static MachineProfile profileFromQuery(const QSqlQuery& q);
};
