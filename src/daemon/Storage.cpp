#include "Storage.hpp"

#include <filesystem>
#include <sstream>
#include <vector>
#include <ErrCode.hpp>
#include <Log.hpp>
#include <SqlDB.hpp>

namespace elastos {

/* =========================================== */
/* === static variables initialize =========== */
/* =========================================== */
const std::string Storage::StorageName = "carrier-group.db";
const int Storage::StorageMessageSize = 100;
const std::string Storage::TableName::Member = "member";
const std::string Storage::TableName::Message = "message";
const std::string Storage::Column::Member = "userid, uptime, name, status";
const std::string Storage::Column::Message = "timestamp, sender, content";
const std::string Storage::MemberStatus::Admin = "'admin'";
const std::string Storage::MemberStatus::Member = "'member'";
const std::string Storage::MemberStatus::Blocked = "'blocked'";

/* =========================================== */
/* === static function implement ============= */
/* =========================================== */

/* =========================================== */
/* === class public function implement  ====== */
/* =========================================== */
int Storage::mount(const std::string& dir)
{
    CHECK_ASSERT(database == nullptr, ErrCode::SqlDbMultiMount);

    try {
        database = std::make_shared<SqlDB::Database>(std::filesystem::path(dir) / StorageName,
                                                     SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

        SQLite::Transaction transaction(*database);

        std::string sql = makeCreateSql(TableName::Member, Column::Member, {
            "TEXT UNIQUE NOT NULL",
            "INTEGER NOT NULL",
            "TEXT",
            "TEXT",
        });
        database->exec(sql);

        sql = makeCreateSql(TableName::Message, Column::Message, {
            "INTEGER NOT NULL",
            "TEXT NOT NULL",
            "TEXT",
        });
        database->exec(sql);

        transaction.commit();
    } catch (SqlDB::Exception& e) {
        Log::E(Log::TAG, "sqldb exception: (%d/%d)%s",
                         e.getErrorCode(), e.getExtendedErrorCode(), e.getErrorStr());
        CHECK_ERROR(ErrCode::SqlDbError);
    } catch (std::exception& e) {
        Log::E(Log::TAG, "sqldb exception: %d", e.what());
        CHECK_ERROR(ErrCode::SqlDbError);
    }

    return 0;
}

int Storage::unmount()
{
    database.reset();
    return 0;
}

int Storage::isMounted()
{
    return (database != nullptr);
}

int64_t Storage::uptime(const std::string& userId)
{
    CHECK_ASSERT(database, ErrCode::SqlDbNotMount);

    int64_t uptime = ErrCode::UnknownError;
    try {
        std::string sql = makeQuerySql(TableName::Member, "uptime",
                                       {"userid='" + userId + "'"});
        SqlDB::Statement query(*database, sql);
        query.executeStep();
        uptime = query.getColumn(0);
        CHECK_ERROR(uptime);
    } catch (SqlDB::Exception& e) {
        Log::E(Log::TAG, "sqldb exception: (%d/%d)%s",
                         e.getErrorCode(), e.getExtendedErrorCode(), e.getErrorStr());
        CHECK_ERROR(ErrCode::SqlDbError);
    } catch (std::exception& e) {
        Log::E(Log::TAG, "sqldb exception: %d", e.what());
        CHECK_ERROR(ErrCode::SqlDbError);
    }

    return uptime;

}

int Storage::isOwner(const std::string& userId)
{
    CHECK_ASSERT(database, ErrCode::SqlDbNotMount);

    try {
        std::string sql = makeQuerySql(TableName::Member, "count(*)",
                                       {
                                           "userid='" + userId + "'",
                                           "id=1",
                                       });
        SqlDB::Statement query(*database, sql);
        query.executeStep();
        int count = query.getColumn(0);
        if(count > 0) {
            return 0;
        }
    } catch (SqlDB::Exception& e) {
        Log::E(Log::TAG, "sqldb exception: (%d/%d)%s",
                         e.getErrorCode(), e.getExtendedErrorCode(), e.getErrorStr());
        CHECK_ERROR(ErrCode::SqlDbError);
    } catch (std::exception& e) {
        Log::E(Log::TAG, "sqldb exception: %d", e.what());
        CHECK_ERROR(ErrCode::SqlDbError);
    }

    return ErrCode::NotMatchError;

}

int Storage::isAdmin(const std::string& userId)
{
    CHECK_ASSERT(database, ErrCode::SqlDbNotMount);

    try {
        std::string sql = makeQuerySql(TableName::Member, "count(*)",
                                       {
                                           "userid='" + userId + "'",
                                           "id=1 OR status=" + MemberStatus::Admin,
                                       });
        SqlDB::Statement query(*database, sql);
        query.executeStep();
        int count = query.getColumn(0);
        if(count > 0) {
            return 0;
        }
    } catch (SqlDB::Exception& e) {
        Log::E(Log::TAG, "sqldb exception: (%d/%d)%s",
                         e.getErrorCode(), e.getExtendedErrorCode(), e.getErrorStr());
        CHECK_ERROR(ErrCode::SqlDbError);
    } catch (std::exception& e) {
        Log::E(Log::TAG, "sqldb exception: %d", e.what());
        CHECK_ERROR(ErrCode::SqlDbError);
    }

    return ErrCode::NotMatchError;

}

int Storage::isMember(const std::string& userId)
{
    CHECK_ASSERT(database, ErrCode::SqlDbNotMount);

    try {
        std::string sql = makeQuerySql(TableName::Member, "count(*)",
                                       {
                                           "userid='" + userId + "'",
                                           "status!=" + MemberStatus::Blocked,
                                       });
        SqlDB::Statement query(*database, sql);
        query.executeStep();
        int count = query.getColumn(0);
        if(count > 0) { // not found blocked member, allow to access.
            return 0;
        }
    } catch (SqlDB::Exception& e) {
        Log::E(Log::TAG, "sqldb exception: (%d/%d)%s",
                         e.getErrorCode(), e.getExtendedErrorCode(), e.getErrorStr());
        CHECK_ERROR(ErrCode::SqlDbError);
    } catch (std::exception& e) {
        Log::E(Log::TAG, "sqldb exception: %d", e.what());
        CHECK_ERROR(ErrCode::SqlDbError);
    }

    return ErrCode::NotMatchError;

}

int Storage::updateMember(const std::string& userId,
                          int64_t uptime,
                          const std::string& name,
                          const std::string& status)
{
    CHECK_ASSERT(database, ErrCode::SqlDbNotMount);

    try {
        std::stringstream values;
        values << "'" << userId << "'," << uptime << ",'" << name << "'," << status;
        std::string sql = makeUpdateSql(TableName::Member, Column::Member, values.str(),
                                        "userid", {"uptime", "name", "status"});
        database->exec(sql);
    } catch (SqlDB::Exception& e) {
        Log::E(Log::TAG, "sqldb exception: (%d/%d)%s",
                         e.getErrorCode(), e.getExtendedErrorCode(), e.getErrorStr());
        CHECK_ERROR(ErrCode::SqlDbError);
    } catch (std::exception& e) {
        Log::E(Log::TAG, "sqldb exception: %d", e.what());
        CHECK_ERROR(ErrCode::SqlDbError);
    }

    return 0;
}

int Storage::updateMember(const std::string& userId,
                          int64_t uptime)
{
    CHECK_ASSERT(database, ErrCode::SqlDbNotMount);

    try {
        std::stringstream values;
        values << "'" << userId << "'," << uptime << ",'',''";
        std::string sql = makeUpdateSql(TableName::Member, Column::Member, values.str(),
                                        "userid", {"uptime"});
        database->exec(sql);
    } catch (SqlDB::Exception& e) {
        Log::E(Log::TAG, "sqldb exception: (%d/%d)%s",
                         e.getErrorCode(), e.getExtendedErrorCode(), e.getErrorStr());
        CHECK_ERROR(ErrCode::SqlDbError);
    } catch (std::exception& e) {
        Log::E(Log::TAG, "sqldb exception: %d", e.what());
        CHECK_ERROR(ErrCode::SqlDbError);
    }

    return 0;
}

int Storage::updateMember(const std::string& userId,
                          const std::string status)
{
    CHECK_ASSERT(database, ErrCode::SqlDbNotMount);

    try {
        std::stringstream values;
        values << "'" << userId << "',0,''," << status;
        std::string sql = makeUpdateSql(TableName::Member, Column::Member, values.str(),
                                        "userid", {"status"});
        database->exec(sql);
    } catch (SqlDB::Exception& e) {
        Log::E(Log::TAG, "sqldb exception: (%d/%d)%s",
                         e.getErrorCode(), e.getExtendedErrorCode(), e.getErrorStr());
        CHECK_ERROR(ErrCode::SqlDbError);
    } catch (std::exception& e) {
        Log::E(Log::TAG, "sqldb exception: %d", e.what());
        CHECK_ERROR(ErrCode::SqlDbError);
    }

    return 0;
}

int Storage::updateMessage(const MessageInfo& info)
{
    CHECK_ASSERT(database, ErrCode::SqlDbNotMount);

    try {
        std::stringstream values;
        values << info.timestamp << ",'" << info.sender << "','" << info.content << "'";
        std::string sql = makeUpdateSql(TableName::Message, Column::Message, values.str());
        database->exec(sql);
    } catch (SqlDB::Exception& e) {
        Log::E(Log::TAG, "sqldb exception: (%d/%d)%s",
                         e.getErrorCode(), e.getExtendedErrorCode(), e.getErrorStr());
        CHECK_ERROR(ErrCode::SqlDbError);
    } catch (std::exception& e) {
        Log::E(Log::TAG, "sqldb exception: %d", e.what());
        CHECK_ERROR(ErrCode::SqlDbError);
    }

    return 0;
}

int Storage::findMessages(int64_t startTime, int count,
                          const std::string& ignoreId,
                          std::vector<MessageInfo>& list)
{
    CHECK_ASSERT(database, ErrCode::SqlDbNotMount);

    int found = 0;
    list.clear();

    try {
        std::string sql = makeQuerySql(TableName::Message, Column::Message,
                                       {
                                           "sender!='" + ignoreId + "'",
                                           "timestamp>" + std::to_string(startTime),
                                       },
                                       "timestamp", count);
        SqlDB::Statement query(*database, sql);
        while(query.executeStep()) {
            int64_t timestamp = query.getColumn(0);
            const char* sender = query.getColumn(1);
            const char* content = query.getColumn(2);
            list.push_back({timestamp, sender, content});
            found++;
        }
    } catch (SqlDB::Exception& e) {
        Log::E(Log::TAG, "sqldb exception: (%d/%d)%s",
                         e.getErrorCode(), e.getExtendedErrorCode(), e.getErrorStr());
        CHECK_ERROR(ErrCode::SqlDbError);
    } catch (std::exception& e) {
        Log::E(Log::TAG, "sqldb exception: %d", e.what());
        CHECK_ERROR(ErrCode::SqlDbError);
    }

    return found;

}

/* =========================================== */
/* === class protected function implement  === */
/* =========================================== */

/* =========================================== */
/* === class private function implement  ===== */
/* =========================================== */
std::string Storage::makeCreateSql(const std::string& table,
                                   const std::string& columns,
                                   const std::vector<std::string>& props)
{
    std::stringstream sqlStream;

    sqlStream << "CREATE TABLE IF NOT EXISTS " << table;
    sqlStream << "(id INTEGER PRIMARY KEY AUTOINCREMENT, " << columns;
    sqlStream << ");";

    auto sql = sqlStream.str();

    int commaIdx = sql.find_first_of(',');
    for(const auto& it: props) {
        commaIdx = sql.find_first_of(',', commaIdx + 1);
        if(commaIdx == std::string::npos) {
            commaIdx = sql.find_last_of(')');
        }
        sql.insert(commaIdx, std::string{' '} + it);
        commaIdx += it.length() + 1;
    }
    Log::V(Log::TAG, "make create table sql: %s", sql.c_str());

    return sql;
}

std::string Storage::makeUpdateSql(const std::string& table,
                                   const std::string& columns,
                                   const std::string& values,
                                   const std::string& check,
                                   const std::vector<std::string>& updates)
{
    std::stringstream sqlStream;

    sqlStream << "INSERT INTO " << table << " (" << columns << ") VALUES (" << values << ")";
    if(check.empty() == false) {
        sqlStream << " ON CONFLICT(" << check << ") DO UPDATE SET ";
        for(auto idx = 0; idx < updates.size(); idx++) {
            if(idx > 0) {
                sqlStream << ",";
            }
            sqlStream << updates[idx] << "=excluded." << updates[idx];
        }
    }
    sqlStream << ";";
    
    auto sql = sqlStream.str();
    Log::V(Log::TAG, "make update table sql: %s", sql.c_str());

    return sql;
}

std::string Storage::makeQuerySql(const std::string& table,
                                  const std::string& columns,
                                  const std::vector<std::string>& conditions,
                                  const std::string& order,
                                  int limit)
{
    std::stringstream sqlStream;

    sqlStream << "SELECT " << columns << " FROM " << table;
    for(int idx = 0; idx < conditions.size(); idx++) {
        if(idx == 0) {
            sqlStream << " WHERE ";
        } else {
            sqlStream << " AND ";
        }
        sqlStream << "(" << conditions[idx] << ")";
    }
    if(order.empty() == false) {
        sqlStream << " ORDER BY " << order;
    }
    if(limit > 0) {
        sqlStream << " LIMIT " << limit;
    }
    sqlStream << ";";
    
    auto sql = sqlStream.str();
    Log::V(Log::TAG, "make query table sql: %s", sql.c_str());

    return sql;
}

} // namespace elastos
