#pragma once

#include <QSqlDatabase>

class IDatabaseProvider {
public:
    virtual ~IDatabaseProvider() = default;

    virtual QSqlDatabase& db() = 0;
};