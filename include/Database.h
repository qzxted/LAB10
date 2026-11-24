#pragma once

#include "Call.h"
#include "CallStateHistory.h"
#include "Operator.h"

#include <sqlite_orm/sqlite_orm.h>
#include <string>

inline auto createStorage(const std::string& filename)
{
    using namespace sqlite_orm;

    return make_storage(
        filename,
        make_table(
            "Operator",
            make_column("id", &Operator::id, primary_key().autoincrement()),
            make_column("fullName", &Operator::fullName),
            make_column("department", &Operator::department),
            make_column("status", &Operator::status)
        ),
        make_table(
            "Call",
            make_column("id", &Call::id, primary_key().autoincrement()),
            make_column("operatorId", &Call::operatorId),
            make_column("callDate", &Call::callDate),
            make_column("phoneNumber", &Call::phoneNumber),
            make_column("durationSec", &Call::durationSec),
            make_column("status", &Call::status),
            foreign_key(&Call::operatorId).references(&Operator::id)
        ),
        make_table(
            "CallStateHistory",
            make_column("id", &CallStateHistory::id, primary_key().autoincrement()),
            make_column("callId", &CallStateHistory::callId),
            make_column("fromState", &CallStateHistory::fromState),
            make_column("toState", &CallStateHistory::toState),
            make_column("changedAt", &CallStateHistory::changedAt),
            make_column("note", &CallStateHistory::note),
            foreign_key(&CallStateHistory::callId).references(&Call::id)
        )
    );
}

using CallCenterStorage = decltype(createStorage(""));