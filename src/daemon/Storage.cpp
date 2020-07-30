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
std::shared_ptr<Storage> Storage::gStorage;
std::recursive_mutex Storage::gMutex = {};
const std::string Storage::gStorageName = "carrier-group.db";
const std::string Storage::Sql::CreateTable = "CREATE TABLE IF NOT EXISTS ";
const std::string Storage::Sql::Update = "INSERT OR REPLACE INTO ";
const std::string Storage::Sql::Count = "SELECT count(*) FROM ";
const std::string Storage::TableName::Member = "'member'";
const std::string Storage::TableName::Message = "'message'";
const std::string Storage::MemberStatus::Admin = "'admin'";
const std::string Storage::MemberStatus::Blocked = "'blocked'";

/* =========================================== */
/* === static function implement ============= */
/* =========================================== */
std::shared_ptr<Storage> Storage::GetInstance()
{
    if(gStorage != nullptr) {
        return gStorage;
    }

    std::lock_guard<std::recursive_mutex> lg(gMutex);
    if(gStorage != nullptr) {
        return gStorage;
    }

    struct Impl: Storage {
    };
    auto gStorage = std::make_shared<Impl>();

    return gStorage;
}

/* =========================================== */
/* === class public function implement  ====== */
/* =========================================== */
int Storage::mount(const std::string& dir)
{
    CHECK_ASSERT(database == nullptr, ErrCode::SqlDbMultiMount);

    try {
        database = std::make_shared<SqlDB::Database>(std::filesystem::path(dir) / gStorageName,
                                                     SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

        SQLite::Transaction transaction(*database);

        std::stringstream sql;
        sql << Sql::CreateTable << TableName::Member << "("
            << "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            << "userid TEXT UNIQUE NOT NULL,"
            << "uptime INTEGER NOT NULL,"
            << "name TEXT,"
            << "status TEXT DEFAULT FALSE);";
        sql << Sql::CreateTable << TableName::Message << "("
            << "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            << "timestamp INTEGER NOT NULL,"
            << "sender TEXT NOT NULL,"
            << "content TEXT);";
        Log::V(Log::TAG, "SQL: %s", sql.str().c_str());
        database->exec(sql.str());

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


int Storage::isOwner(const std::string& userId)
{
    CHECK_ASSERT(database, ErrCode::SqlDbNotMount);

    try {
        std::stringstream sql;
        sql << Sql::Count << TableName::Member
            << " WHERE userid = '" << userId << "' AND (id=1 OR status=" << MemberStatus::Admin << ");";
        Log::V(Log::TAG, "SQL: %s", sql.str().c_str());
        SqlDB::Statement query(*database, sql.str());
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

int Storage::accessible(const std::string& userId)
{
    CHECK_ASSERT(database, ErrCode::SqlDbNotMount);

    try {
        std::stringstream sql;
        sql << Sql::Count << TableName::Member
            << " WHERE userid='" << userId << "' AND status=" << MemberStatus::Blocked << ";";
        Log::V(Log::TAG, "SQL: %s", sql.str().c_str());
        SqlDB::Statement query(*database, sql.str());
        query.executeStep();
        int count = query.getColumn(0);
        if(count <= 0) { // not found blocked member, allow to access.
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

int Storage::update(const std::string& userId,
                    int64_t timestamp,
                    const std::string& name,
                    const std::string& status)
{
    CHECK_ASSERT(database, ErrCode::SqlDbNotMount);

    try {
        std::stringstream sql;
        sql << Sql::Update << TableName::Member 
            << "(userid, uptime, name, status) VALUES ("
            << "'" << userId << "'," << timestamp << ",'" << name << "','" << status << "');";
        Log::V(Log::TAG, "SQL: %s", sql.str().c_str());
        database->exec(sql.str());
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

/* =========================================== */
/* === class protected function implement  === */
/* =========================================== */

/* =========================================== */
/* === class private function implement  ===== */
/* =========================================== */


} // namespace elastos
