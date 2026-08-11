#pragma once

#include <QList>
#include <QPair>
#include "ProcessProfile.h"

class IDatabaseProvider;
class QSqlQuery;

class ProcessRepository {
public:
    explicit ProcessRepository(IDatabaseProvider& provider);
    QList<QPair<int, QString>> listAllName() const;
    QList<ProcessProfile> listAll() const;
    ProcessProfile        getById(int id) const;
    bool                  save(ProcessProfile& profile);
    ProcessProfile        copy(int id);
    bool                  remove(int id);

private:
    IDatabaseProvider& m_provider;

    static ProcessProfile profileFromQuery(const QSqlQuery& q);
};
