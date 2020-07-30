/**
 * @file	CarrierHandler.hpp
 * @brief	CarrierHandler
 * @details	
 *
 * @author	xxx
 * @author	<xxx@xxx.com>
 * @copyright	(c) 2012 xxx All rights reserved.
 **/

#ifndef _ELASTOS_STORAGE_HPP_
#define _ELASTOS_STORAGE_HPP_

#include <memory>
#include <mutex>
#include <SqlDB.hpp>

namespace elastos {

class Storage {
public:
    /*** type define ***/

    /*** static function and variable ***/
    static std::shared_ptr<Storage> GetInstance();

    /*** class function and variable ***/
    explicit Storage() = default;
    virtual ~Storage() = default;

    int mount(const std::string& dir);
    int unmount();
    int isMounted();

    int isOwner(const std::string& userId);
    int accessible(const std::string& userId);

    int update(const std::string& userId,
               int64_t timestamp,
               const std::string& name,
               const std::string& status = "");

protected:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/

private:
    /*** type define ***/
    struct Sql {
        static const std::string CreateTable;
        static const std::string Update;
        static const std::string Count;
    };
    struct TableName {
        static const std::string Member;
        static const std::string Message;
    };
    struct MemberStatus {
        static const std::string Admin;
        static const std::string Blocked;
    };

    /*** static function and variable ***/
    static std::recursive_mutex gMutex;
    static std::shared_ptr<Storage> gStorage;
    static const std::string gStorageName;

    /*** class function and variable ***/
    std::shared_ptr<SqlDB::Database> database;

}; // class Storage

} // namespace elastos

#endif /* _ELASTOS_STORAGE_HPP_ */