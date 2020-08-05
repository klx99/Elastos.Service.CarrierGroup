#include "CmdParser.hpp"

#include <string>
#include <sstream>
#include <vector>
#include <Carrier.hpp>
#include <DateTime.hpp>
#include <ErrCode.hpp>
#include <Log.hpp>

namespace elastos {

/* =========================================== */
/* === static variables initialize =========== */
/* =========================================== */
std::shared_ptr<CmdParser> CmdParser::CmdParserInstance;
std::recursive_mutex CmdParser::CmdMutex = {};
const std::string CmdParser::PromptAccessForbidden = "Access Forbidden!";
const std::string CmdParser::PromptBadCommand = "Bad Command!"
                                                "\nPlease use '/help' to get usage of all commands.";
const std::string CmdParser::PromptBadArguments = "Bad Arguments!";
const std::string CmdParser::PromptKicked = "You are kicked by group admin!";

/* =========================================== */
/* === static function implement ============= */
/* =========================================== */
std::shared_ptr<CmdParser> CmdParser::GetInstance()
{
    if(CmdParserInstance != nullptr) {
        return CmdParserInstance;
    }

    std::lock_guard<std::recursive_mutex> lg(CmdMutex);
    if(CmdParserInstance != nullptr) {
        return CmdParserInstance;
    }

    struct Impl: CmdParser {
    };
    CmdParserInstance = std::make_shared<Impl>();

    return CmdParserInstance;
}

/* =========================================== */
/* === class public function implement  ====== */
/* =========================================== */
void CmdParser::setStorageDir(const std::string& dir)
{
    dataDir = dir;
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

    if(storage.isMounted() == false) {
        int rc = storage.mount(dataDir);
        CHECK_ERROR(rc);
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

/* =========================================== */
/* === class private function implement  ===== */
/* =========================================== */
CmdParser::CmdParser()
    : taskThread(std::make_unique<ThreadPool>("parser-thread"))
    , dataDir()
    , storage()
{
    using namespace std::placeholders;

    cmdInfoList = std::vector<CommandInfo>{
        {
            Cmd::Help,
            CommandInfo::Performer::Admin,
            std::bind(&CmdParser::onHelp, this, _1, _2, _3, _4),
            "[" + Cmd::Help + "] Print help usages."
        }, {
            Cmd::ListFriend,
            CommandInfo::Performer::Admin,
            std::bind(&CmdParser::onListFriend, this, _1, _2, _3, _4),
            "[" + Cmd::ListFriend + "] List all friends."
        }, {
            Cmd::AddFriend,
            CommandInfo::Performer::Anyone,
            std::bind(&CmdParser::onAddFriend, this, _1, _2, _3, _4),
            "[" + Cmd::AddFriend + "] Received a friend request."
        }, {
            Cmd::InviteFriend,
            CommandInfo::Performer::Admin,
            std::bind(&CmdParser::onInviteFriend, this, _1, _2, _3, _4),
            "[" + Cmd::InviteFriend + " address (brief)] Invite a friend into group."
        }, {
            Cmd::KickFriend,
            CommandInfo::Performer::Admin,
            std::bind(&CmdParser::onKickFriend, this, _1, _2, _3, _4),
            "[" + Cmd::KickFriend + " id] Kick a friend from group."
        }, {
            Cmd::ForwardMessage,
            CommandInfo::Performer::Member,
            std::bind(&CmdParser::onForwardMessage, this, _1, _2, _3, _4),
            "[" + Cmd::ForwardMessage + "] Forward message to all online peer."
        },
    };
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
        if(cmdInfo.cmd == "-") {
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

int CmdParser::onListFriend(const std::weak_ptr<Carrier>& carrier,
                            const std::vector<std::string>& args,
                            const std::string& controller, int64_t timestamp)
{
    auto carrierPtr = carrier.lock();
    CHECK_ASSERT(carrierPtr, ErrCode::PointerReleasedError);

    std::vector<std::string> friendList;
    int rc = carrierPtr->getFriendList(false, friendList);
    CHECK_ERROR(rc);

    std::stringstream buf;
    for(const auto& it: friendList) {
        buf << it << '\n';
    }
    rc = carrierPtr->sendMessage(controller, buf.str());
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

    rc = storage.updateMember(controller, timestamp, friendName);
    CHECK_ERROR(rc);

    return 0;
}

int CmdParser::onInviteFriend(const std::weak_ptr<Carrier>& carrier,
                              const std::vector<std::string>& args,
                              const std::string& controller, int64_t timestamp)
{
    auto carrierPtr = carrier.lock();
    CHECK_ASSERT(carrierPtr, ErrCode::PointerReleasedError);

    if(args.size() < 1) {
        carrierPtr->sendMessage(controller, PromptBadArguments);
        CHECK_ERROR(ErrCode::InvalidArgument);
    }
    const auto address = args[0];
    std::string brief = "Hello!";
    if(args.size() >= 2) {
        brief = args[1];
    }

    if(Carrier::CheckAddress(address) == false) {
        carrierPtr->sendMessage(controller, PromptBadArguments);
        CHECK_ERROR(ErrCode::InvalidArgument);
    }

    int rc = carrierPtr->addFriend(address, brief);
    CHECK_ERROR(rc);

    std::string friendId;
    rc = Carrier::GetUsrIdByAddress(address, friendId);
    CHECK_ERROR(rc);

    std::string friendName; 
    rc = carrierPtr->getFriendNameById(controller, friendName);
    CHECK_ERROR(rc);

    rc = storage.updateMember(friendId, 0 /*timestamp*/, friendName, Storage::MemberStatus::Member);
    CHECK_ERROR(rc);

    return 0;
}

int CmdParser::onKickFriend(const std::weak_ptr<Carrier>& carrier,
                            const std::vector<std::string>& args,
                            const std::string& controller, int64_t timestamp)
{
    auto carrierPtr = carrier.lock();
    CHECK_ASSERT(carrierPtr, ErrCode::PointerReleasedError);

    if(args.size() < 1) {
        carrierPtr->sendMessage(controller, PromptBadArguments);
        CHECK_ERROR(ErrCode::InvalidArgument);
    }
    const auto memberId = args[0];

    carrierPtr->sendMessage(memberId, PromptKicked); // ignore return value

    int rc = carrierPtr->removeFriend(memberId);
    CHECK_ERROR(rc);

    rc = storage.updateMember(memberId, Storage::MemberStatus::Blocked);
    CHECK_ERROR(rc);

    return 0;
}

int CmdParser::onForwardMessage(const std::weak_ptr<Carrier>& carrier,
                                const std::vector<std::string>& args,
                                const std::string& controller, int64_t timestamp)
{
    if(args.size() == 1) {
        const std::string& message = args[0];
        int rc = storage.updateMessage({timestamp, controller, message});
        CHECK_ERROR(rc);
    }

    taskThread->post(std::bind(&CmdParser::forwardMsgToAllFriends, this, carrier));

    return 0;
}

int CmdParser::checkPerformer(const std::string& friendId,
                              const CommandInfo::Performer& performer)
{
    int rc;

    switch (performer) {
    case CommandInfo::Performer::Owner:
        rc = storage.isOwner(friendId);
        break;
    case CommandInfo::Performer::Admin:
        rc = storage.isAdmin(friendId);
        break;
    case CommandInfo::Performer::Member:
        rc = storage.isMember(friendId);
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

int CmdParser::forwardMsgToAllFriends(const std::weak_ptr<Carrier>& carrier)
{
    auto carrierPtr = carrier.lock();
    CHECK_ASSERT(carrierPtr, ErrCode::PointerReleasedError);

    std::vector<std::string> friendList;
    int rc = carrierPtr->getFriendList(true, friendList);
    CHECK_ERROR(rc);

    for(const auto& friendId: friendList) {
        int rc = forwardMsgToFriend(carrier, friendId);
        if(rc < 0) {
            Log::W(Log::TAG, "Failed to forward message to %s", friendId.c_str());
            continue;
        }
    }

    return 0;
}

int CmdParser::forwardMsgToFriend(const std::weak_ptr<Carrier>& carrier,
                                  const std::string& friendId)
{
    auto carrierPtr = carrier.lock();
    CHECK_ASSERT(carrierPtr, ErrCode::PointerReleasedError);

    int found;
    int64_t uptime = storage.uptime(friendId);
    CHECK_ERROR(uptime);
    constexpr const int reservedId = 5;
    constexpr const int count = 10;

    do {
        std::vector<Storage::MessageInfo> msgList;
        found = storage.findMessages(uptime, 10, friendId, msgList);
        CHECK_ERROR(found);

        for(auto& info: msgList) {
            info.sender.replace(info.sender.begin() + reservedId, info.sender.end() - reservedId, "...");
            std::string msg = "[" + DateTime::NsToString(info.timestamp) + "]"
                            + "\n[" + info.sender + "]:"
                            + "\n" + info.content;
            int rc = carrierPtr->sendMessage(friendId, msg);
            if(rc < 0) {
                Log::W(Log::TAG, "Failed to send message at '%lld' to %s.", info.timestamp, friendId.c_str());
                break;
            }

            uptime = info.timestamp;
        }
    } while(found > 0);

    int rc = storage.updateMember(friendId, uptime);
    CHECK_ERROR(rc);

    return 0;
}

std::string CmdParser::trim(const std::string &str)
{
   auto wsfront=std::find_if_not(str.begin(),str.end(),[](int c){return std::isspace(c);});
   auto wsback=std::find_if_not(str.rbegin(),str.rend(),[](int c){return std::isspace(c);}).base();
   return (wsback<=wsfront ? std::string() : std::string(wsfront,wsback));
}



} // namespace elastos
