#include "CmdParser.hpp"

#include <string>
#include <sstream>
#include <vector>
#include <Carrier.hpp>
#include <CompatibleFileSystem.hpp>
#include <DateTime.hpp>
#include <ErrCode.hpp>
#include <GroupCmdParser.hpp>
#include <Log.hpp>
#include <ManagerCmdParser.hpp>
#include <OptParser.hpp>
#include <Process.hpp>

namespace elastos {

/* =========================================== */
/* === static variables initialize =========== */
/* =========================================== */
const std::string CmdParser::CarrierAddressName = "carrier-address";

const std::string CmdParser::PromptAccessForbidden = "Access Forbidden!";
const std::string CmdParser::PromptBadCommand = "Bad Command!"
                                                "\nPlease use '/help' to get usage of all commands.";
const std::string CmdParser::PromptBadArguments = "Bad Arguments!";

/* =========================================== */
/* === static function implement ============= */
/* =========================================== */
std::shared_ptr<CmdParser> CmdParser::Factory::Create()
{
    std::shared_ptr<CmdParser> cmdParser;

    bool isMgr = OptParser::GetInstance()->isManager();
    if(isMgr == true) {
        struct Impl: ManagerCmdParser {
        };
        cmdParser = std::make_shared<Impl>();
    } else {
        struct Impl: GroupCmdParser {
        };
        cmdParser = std::make_shared<Impl>();
    }

    return cmdParser;
}

/* =========================================== */
/* === class public function implement  ====== */
/* =========================================== */
int CmdParser::config(const std::weak_ptr<Carrier>& carrier)
{
    this->taskThread = std::make_shared<ThreadPool>("parser-thread");
    this->storage = std::make_shared<Storage>();
    this->cmdInfoList = std::move(getCmdInfoList());

    int rc = storage->mount(OptParser::GetInstance()->getDataDir());
    CHECK_ERROR(rc);

    auto carrierPtr = carrier.lock();
    CHECK_ASSERT(carrierPtr, ErrCode::PointerReleasedError);

    std::string address;
    rc = carrierPtr->getAddress(address);
    CHECK_ERROR(rc);

    auto addrFilePath = std::filesystem::path {OptParser::GetInstance()->getDataDir()};
    std::ofstream addrFile {addrFilePath / CarrierAddressName};
    addrFile << address;
    addrFile.flush();

    return 0;
}

int CmdParser::parse(const std::weak_ptr<Carrier>& carrier,
                     const std::string& cmdline,
                     const std::string& controller, int64_t timestamp)
{
    Log::D(Log::TAG, "%s cmdline=%s", __PRETTY_FUNCTION__, cmdline.c_str());
    std::string cmd;
    std::vector<std::string> args;

    auto trimLine = trim(cmdline);
    int cmdIndex = trimLine.find(' ');
    if(cmdIndex == std::string::npos) {
        cmdIndex =  trimLine.find(':');
    }
    if(cmdIndex > 0) {
        cmd = trimLine.substr(0, cmdIndex);
        auto argsLine = trimLine.substr(cmdIndex + 1);
        int argIndex = argsLine.find(' ');
        if(cmdIndex == std::string::npos) {
            argIndex = argsLine.find(':');
        }
        if(argIndex > 0) {
            args.push_back(argsLine.substr(0, argIndex));
            args.push_back(argsLine.substr(argIndex + 1));
        } else {
            args.push_back(argsLine);
        }
    } else {
        cmd = trimLine;
    }

    taskThread->post(std::bind(&CmdParser::dispatch, this, carrier, cmd, args, controller, timestamp));

    return 0;
}

int CmdParser::dispatch(const std::weak_ptr<Carrier>& carrier,
                        std::string cmd, const std::vector<std::string>& args,
                        const std::string& controller, int64_t timestamp)
{
    Log::V(Log::TAG, "dispach cmd: %s", cmd.c_str());
    for(const auto& it: args) {
        Log::V(Log::TAG, "arg: %s", it.c_str());
    }

    int rc = ErrCode::UnimplementedError;
    for(const auto& cmdInfo : cmdInfoList) {
        // if(cmdInfo.cmd == ' '
        // || cmdInfo.func == nullptr) {
        //     continue;
        // }
        if(cmd != cmdInfo.cmd) {
            continue;
        }

        rc = checkPerformer(controller, cmdInfo.performer);
        if(rc < 0) {
            Log::W(Log::TAG, "%s %s want to exec command %s(perform=%d).",
                             PromptAccessForbidden.c_str(), controller.c_str(), cmdInfo.cmd.c_str(), cmdInfo.performer);
            auto carrierPtr = carrier.lock();
            CHECK_ASSERT(carrierPtr, ErrCode::PointerReleasedError);
            rc = carrierPtr->sendMessage(controller, PromptAccessForbidden);
            CHECK_ERROR(rc);
        }

        rc = cmdInfo.func(carrier, args, controller, timestamp);
        CHECK_ERROR(rc);

        break;
    }
    if(rc == ErrCode::UnimplementedError) {
        rc = onUnimplemented(carrier, args, controller, timestamp);
        CHECK_ERROR(rc);
    }

    return 0;
}

/* =========================================== */
/* === class protected function implement  === */
/* =========================================== */
std::vector<CmdParser::CommandInfo> CmdParser::getCmdInfoList()
{
    using namespace std::placeholders;
    std::vector<CommandInfo> cmdInfoList;

    cmdInfoList = {
        {
            Cmd::Help,
            CommandInfo::Performer::Anyone,
            std::bind(&CmdParser::onHelp, this, _1, _2, _3, _4),
            "[" + Cmd::Help + "] Print help usages."
        }, {
            Cmd::AddFriend, // process carrier friend request.
            CommandInfo::Performer::Anyone,
            std::bind(&GroupCmdParser::onAddFriend, this, _1, _2, _3, _4),
            ""
        },
    };

    return cmdInfoList;
}

std::shared_ptr<ThreadPool> CmdParser::getTaskThread()
{
    return taskThread;
}

std::shared_ptr<Storage> CmdParser::getStorage()
{
    return storage;
}

/* =========================================== */
/* === class private function implement  ===== */
/* =========================================== */
int CmdParser::onIgnore(const std::weak_ptr<Carrier>& carrier,
                        const std::vector<std::string>& args,
                        const std::string& controller, int64_t timestamp)
{
    return 0;
}

int CmdParser::onUnimplemented(const std::weak_ptr<Carrier>& carrier,
                               const std::vector<std::string>& args,
                               const std::string& controller, int64_t timestamp)
{
    auto carrierPtr = carrier.lock();
    CHECK_ASSERT(carrierPtr, ErrCode::PointerReleasedError);
    int rc = carrierPtr->sendMessage(controller, PromptBadCommand);
    CHECK_ERROR(rc);

    return 0;
}

int CmdParser::onHelp(const std::weak_ptr<Carrier>& carrier,
                      const std::vector<std::string>& args,
                      const std::string& controller, int64_t timestamp)
{
    std::stringstream usage;

    usage << "Usage:" << std::endl;
    for(const auto& cmdInfo : cmdInfoList) {
        if(cmdInfo.usage == "") {
            continue;
        } else if(cmdInfo.cmd == "-") {
            usage << cmdInfo.usage << std::endl;
        } else {
            usage << "  " << cmdInfo.cmd <<  ": " << cmdInfo.usage << std::endl;
        }
    }
    usage << std::endl;

    auto carrierPtr = carrier.lock();
    CHECK_ASSERT(carrierPtr, ErrCode::PointerReleasedError);
    int rc = carrierPtr->sendMessage(controller, usage.str());
    CHECK_ERROR(rc);

    return 0;
}

int CmdParser::onAddFriend(const std::weak_ptr<Carrier>& carrier,
                           const std::vector<std::string>& args,
                           const std::string& controller, int64_t timestamp)
{
    auto carrierPtr = carrier.lock();
    CHECK_ASSERT(carrierPtr, ErrCode::PointerReleasedError);

    int rc = carrierPtr->addFriend(controller);
    CHECK_ERROR(rc);

    std::string friendName; 
    rc = carrierPtr->getFriendNameById(controller, friendName);
    CHECK_ERROR(rc);

    rc = getStorage()->updateMember(controller, timestamp, friendName);
    CHECK_ERROR(rc);

    return 0;
}

int CmdParser::checkPerformer(const std::string& friendId,
                              const CommandInfo::Performer& performer)
{
    int rc;

    switch (performer) {
    case CommandInfo::Performer::Owner:
        rc = storage->isOwner(friendId);
        break;
    case CommandInfo::Performer::Admin:
        rc = storage->isAdmin(friendId);
        break;
    case CommandInfo::Performer::Member:
        rc = storage->isMember(friendId);
        break;
    case CommandInfo::Performer::Anyone:
        rc = 0;
        break;
    default:
        rc = ErrCode::ErrCode::NotMatchError;
        break;
    }

    return rc;
}

std::string CmdParser::trim(const std::string &str)
{
   auto wsfront=std::find_if_not(str.begin(),str.end(),[](int c){return std::isspace(c);});
   auto wsback=std::find_if_not(str.rbegin(),str.rend(),[](int c){return std::isspace(c);}).base();
   return (wsback<=wsfront ? std::string() : std::string(wsfront,wsback));
}



} // namespace elastos
