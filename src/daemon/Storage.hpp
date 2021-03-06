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
#include <string>
#include <vector>
#include <SqlDB.hpp>

namespace elastos {

class Storage {
public:
    /*** type define ***/
    struct MessageInfo {
        int64_t timestamp;
        std::string sender;
        std::string content;
    };

    struct MemberStatus {
        static const std::string Admin;
        static const std::string Member;
        static const std::string Blocked;
    };

    /*** static function and variable ***/

    /*** class function and variable ***/
    explicit Storage() = default;
    virtual ~Storage() = default;

    int mount(const std::string& dir);
    int unmount();
    int isMounted();

    int64_t uptime(const std::string& userId);

    int isOwner(const std::string& userId);
    int isAdmin(const std::string& userId);
    int isMember(const std::string& userId);

    int updateMember(const std::string& userId,
                     int64_t uptime,
                     const std::string& name,
                     const std::string& status = MemberStatus::Member);
    int updateMember(const std::string& userId,
                     int64_t uptime);
    int updateMember(const std::string& userId,
                     const std::string status);

    int updateMessage(const MessageInfo& info);

    int updateManager(int64_t timestamp,
                      const std::string& owner,
                      const std::string& groupAddr,
                      const std::string& groupDir);

    int deleteManager(const std::string& groupAddr);

    int findMessages(int64_t startTime, int count,
                     const std::string& ignoreId,
                     std::vector<MessageInfo>& list);

    int findGroup(const std::string& groupAddr,
                  std::string& groupDir);

    int listGroup(const std::string& owner,
                  std::vector<std::string>& groupList);


protected:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/

private:
    /*** type define ***/
    struct TableName {
        static const std::string Member;
        static const std::string Message;
        static const std::string Manager;
    };
    struct Column {
        static const std::string Member;
        static const std::string Message;
        static const std::string Manager;
    };

    /*** static function and variable ***/
    static const std::string StorageName;
    static const int StorageMessageSize;

    /*** class function and variable ***/
    bool hasOwner();
    std::string makeCreateSql(const std::string& table,
                              const std::string& columns,
                              const std::vector<std::string>& props);
    std::string makeUpdateSql(const std::string& table,
                              const std::string& columns,
                              const std::string& values,
                              const std::string& check = {},
                              const std::vector<std::string>& updates = {});
    std::string makeDeleteSql(const std::string& table,
                              const std::vector<std::string>& conditions);
    std::string makeQuerySql(const std::string& table,
                             const std::string& columns,
                             const std::vector<std::string>& conditions,
                             const std::string& order = {},
                             int limit = 0);

    std::shared_ptr<SqlDB::Database> database;

}; // class Storage

} // namespace elastos

#endif /* _ELASTOS_STORAGE_HPP_ */