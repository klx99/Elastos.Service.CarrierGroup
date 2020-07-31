/**
 * @file	CarrierHandler.hpp
 * @brief	CarrierHandler
 * @details	
 *
 * @author	xxx
 * @author	<xxx@xxx.com>
 * @copyright	(c) 2012 xxx All rights reserved.
 **/

#ifndef _ELASTOS_CMD_PARSER_HPP_
#define _ELASTOS_CMD_PARSER_HPP_

#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <Storage.hpp>
#include <ThreadPool.hpp>

namespace elastos {

class Carrier;

class CmdParser {
public:
    /*** type define ***/
    struct Cmd {
        inline static const std::string Help = "/help";
        inline static const std::string AddFriend = "/add";
        inline static const std::string AllowFriend = "/allow";
        inline static const std::string InviteFriend = "/invite";
        inline static const std::string ForwardMessage = "/forward";
    };

    /*** static function and variable ***/
    static std::shared_ptr<CmdParser> GetInstance();

    /*** class function and variable ***/
    void setStorageDir(const std::string& dir);
    int parse(const std::weak_ptr<Carrier>& carrier,
              const std::string& cmdline,
              const std::string& controller, int64_t timestamp);
    int dispatch(const std::weak_ptr<Carrier>& carrier,
                 std::string cmd, const std::vector<std::string>& args,
                 const std::string& controller, int64_t timestamp);

protected:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/

private:
    /*** type define ***/
    struct CommandInfo {
        using Processor = int(const std::weak_ptr<Carrier>& carrier,
                              const std::vector<std::string>&,
                              const std::string&, int64_t);
        std::string cmd;
        std::function<Processor> func;
        std::string usage;
    };

    /*** static function and variable ***/
    static std::recursive_mutex CmdMutex;
    static std::shared_ptr<CmdParser> CmdParserInstance;
    static const std::string PromptAccessForbidden;
    static const std::string PromptBadCommand;

    /*** class function and variable ***/
    explicit CmdParser();
    virtual ~CmdParser() = default;

    int onUnimplemented(const std::weak_ptr<Carrier>& carrier,
                        const std::vector<std::string>& args,
                        const std::string& controller, int64_t timestamp);
    int onHelp(const std::weak_ptr<Carrier>& carrier,
               const std::vector<std::string>& args,
               const std::string& controller, int64_t timestamp);
    int onAddFriend(const std::weak_ptr<Carrier>& carrier,
                    const std::vector<std::string>& args,
                    const std::string& controller, int64_t timestamp);
    int onForwardMessage(const std::weak_ptr<Carrier>& carrier,
                         const std::vector<std::string>& args,
                         const std::string& controller, int64_t timestamp);
    
    int forwardMsgToAllFriends(const std::weak_ptr<Carrier>& carrier);
    int forwardMsgToFriend(const std::weak_ptr<Carrier>& carrier, const std::string& friendId);

    std::string trim(const std::string &str);

    std::unique_ptr<ThreadPool> taskThread;
    std::string dataDir;
    Storage storage;
    std::vector<CommandInfo> cmdInfoList;
}; // class CmdParser

} // namespace elastos

#endif /* _ELASTOS_CMD_PARSER_HPP_ */