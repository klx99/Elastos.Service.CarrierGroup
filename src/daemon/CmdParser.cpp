#include "CmdParser.hpp"

#include <sstream>
#include <vector>
#include <ErrCode.hpp>
#include <Log.hpp>

namespace elastos {

/* =========================================== */
/* === static variables initialize =========== */
/* =========================================== */
std::shared_ptr<CmdParser> CmdParser::gCmdParser;
std::recursive_mutex CmdParser::gMutex = {};

/* =========================================== */
/* === static function implement ============= */
/* =========================================== */
std::shared_ptr<CmdParser> CmdParser::GetInstance()
{
    if(gCmdParser != nullptr) {
        return gCmdParser;
    }

    std::lock_guard<std::recursive_mutex> lg(gMutex);
    if(gCmdParser != nullptr) {
        return gCmdParser;
    }

    struct Impl: CmdParser {
    };
    auto gCmdParser = std::make_shared<Impl>();

    return gCmdParser;
}

/* =========================================== */
/* === class public function implement  ====== */
/* =========================================== */
int CmdParser::parse(const std::weak_ptr<Carrier>& carrier,
                     const std::string& cmdline,
                     const std::string& controller, int64_t timestamp)
{
    // Log::D(Log::TAG, "%s cmdline=%s", __PRETTY_FUNCTION__, cmdline.c_str());
    std::string cmd;
    std::vector<std::string> args;

    auto trimLine = trim(cmdline);
    int cmdIndex = trimLine.find(' ');
    if(cmdIndex > 0) {
        cmd = trimLine.substr(0, cmdIndex);
        auto argsLine = trimLine.substr(cmdIndex + 1);
        int argIndex = argsLine.find(' ');
        if(argIndex > 0) {
            args.push_back(argsLine.substr(0, argIndex));
            args.push_back(argsLine.substr(argIndex + 1));
        } else {
            args.push_back(argsLine);
        }
    } else {
        cmd = trimLine;
    }

    int rc = dispatch(carrier, cmd, args, controller, timestamp);
    CHECK_ERROR(rc);

    return 0;
}

int CmdParser::dispatch(const std::weak_ptr<Carrier>& carrier,
                        std::string cmd, const std::vector<std::string>& args,
                        const std::string& controller, int64_t timestamp)
{
    Log::D(Log::TAG, "%s", __PRETTY_FUNCTION__);
    Log::D(Log::TAG, "cmd: %s", cmd.c_str());
    for(const auto& it: args) {
        Log::D(Log::TAG, "arg: %s", it.c_str());
    }

    for(const auto& cmdInfo : cmdInfoList) {
        // if(cmdInfo.cmd == ' '
        // || cmdInfo.func == nullptr) {
        //     continue;
        // }
        if(cmd != cmdInfo.cmd) {
            continue;
        }

        int rc = cmdInfo.func(carrier, args, controller, timestamp);
        CHECK_ERROR(rc);

        break;
    }

    return 0;
}

/* =========================================== */
/* === class protected function implement  === */
/* =========================================== */

/* =========================================== */
/* === class private function implement  ===== */
/* =========================================== */
CmdParser::CmdParser()
{
    using namespace std::placeholders;

    cmdInfoList = std::vector<CommandInfo>{
        {
            Cmd::Help,
            std::bind(&CmdParser::onHelp, this, _1, _2, _3, _4),
            std::string("[") + Cmd::Help + "] Print help usages."
        }, {
            Cmd::RequestFriend,
            std::bind(&CmdParser::onRequestFriend, this, _1, _2, _3, _4),
            std::string("[") + Cmd::RequestFriend + "] Received a friend request."
        // }, {
        //     Cmd::InviteFriend,
        //     std::bind(&CmdParser::onInviteFriend, this, _1, _2, _3),
        //     std::string("[") + Cmd::InviteFriend + "] Invite a friend into group."
        // }, {
        //     Cmd::RelayMessage,
        //     std::bind(&CmdParser::onRelayMessage, this, _1, _2, _3),
        //     std::string("[") + Cmd::RelayMessage + "] Relay message to peer."
        },
    };
}

int CmdParser::onHelp(const std::weak_ptr<Carrier>& carrier,
                      const std::vector<std::string>& args,
                      const std::string& controller, int64_t timestamp)
{
    std::stringstream usage;
        usage << "Usage:" << std::endl;
    for(const auto& cmdInfo : cmdInfoList) {
        if(cmdInfo.cmd == "-") {
            usage << cmdInfo.usage << std::endl;
        } else {
            usage << "  " << cmdInfo.cmd <<  ": " << cmdInfo.usage << std::endl;
        }
    }
    usage << std::endl;

    // response = usage.str();

    return 0;
}

int CmdParser::onRequestFriend(const std::weak_ptr<Carrier>& carrier,
                               const std::vector<std::string>& args,
                               const std::string& controller, int64_t timestamp)
{
    // response = Cmd::AllowFriend;

    return 0;
}

std::string CmdParser::trim(const std::string &str)
{
   auto wsfront=std::find_if_not(str.begin(),str.end(),[](int c){return std::isspace(c);});
   auto wsback=std::find_if_not(str.rbegin(),str.rend(),[](int c){return std::isspace(c);}).base();
   return (wsback<=wsfront ? std::string() : std::string(wsfront,wsback));
}



} // namespace elastos
