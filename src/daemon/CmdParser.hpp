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
    class Factory {
    public:
        static std::shared_ptr<CmdParser> Create();
    };

    struct Cmd {
        inline static const std::string Help = "/help";
        inline static const std::string AddFriend = "/add";
    };

    /*** static function and variable ***/

    /*** class function and variable ***/
    virtual int config(const std::weak_ptr<Carrier>& carrier);

    virtual int parse(const std::weak_ptr<Carrier>& carrier,
                      const std::string& cmdline,
                      const std::string& controller, int64_t timestamp);
    virtual int dispatch(const std::weak_ptr<Carrier>& carrier,
                         std::string cmd, const std::vector<std::string>& args,
                         const std::string& controller, int64_t timestamp);

protected:
    /*** type define ***/
    struct CommandInfo {
        enum class Performer {
            Owner,
            Admin,
            Member,
            Anyone,
        };

        using Processor = int(const std::weak_ptr<Carrier>& carrier,
                              const std::vector<std::string>&,
                              const std::string&, int64_t);
        std::string cmd;
        Performer performer;
        std::function<Processor> func;
        std::string usage;
    };

    /*** static function and variable ***/
    static const std::string CarrierAddressName;

    static const std::string PromptAccessForbidden;
    static const std::string PromptBadCommand;
    static const std::string PromptBadArguments;

    /*** class function and variable ***/
    explicit CmdParser() = default;
    virtual ~CmdParser() = default;

    virtual std::vector<CommandInfo> getCmdInfoList();
    virtual std::shared_ptr<ThreadPool> getTaskThread();
    virtual std::shared_ptr<Storage> getStorage();

private:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/
    int onIgnore(const std::weak_ptr<Carrier>& carrier,
                 const std::vector<std::string>& args,
                 const std::string& controller, int64_t timestamp);
    int onUnimplemented(const std::weak_ptr<Carrier>& carrier,
                        const std::vector<std::string>& args,
                        const std::string& controller, int64_t timestamp);

    int onHelp(const std::weak_ptr<Carrier>& carrier,
               const std::vector<std::string>& args,
               const std::string& controller, int64_t timestamp);
    int onAddFriend(const std::weak_ptr<Carrier>& carrier,
                    const std::vector<std::string>& args,
                    const std::string& controller, int64_t timestamp);

    int checkPerformer(const std::string& friendId, const CommandInfo::Performer& performer);

    std::string trim(const std::string &str);

    std::shared_ptr<ThreadPool> taskThread;
    std::shared_ptr<Storage> storage;
    std::vector<CommandInfo> cmdInfoList;
}; // class CmdParser

} // namespace elastos

#endif /* _ELASTOS_CMD_PARSER_HPP_ */