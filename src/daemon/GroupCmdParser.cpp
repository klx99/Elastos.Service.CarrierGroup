#include "GroupCmdParser.hpp"

#include <string>
#include <sstream>
#include <vector>
#include <Carrier.hpp>
#include <CompatibleFileSystem.hpp>
#include <DateTime.hpp>
#include <ErrCode.hpp>
#include <Log.hpp>
#include <OptParser.hpp>
#include <Process.hpp>

namespace elastos {

/* =========================================== */
/* === static variables initialize =========== */
/* =========================================== */
const std::string GroupCmdParser::PromptKicked = "You are kicked by group admin!";

/* =========================================== */
/* === static function implement ============= */
/* =========================================== */

/* =========================================== */
/* === class public function implement  ====== */
/* =========================================== */

/* =========================================== */
/* === class protected function implement  === */
/* =========================================== */
std::vector<CmdParser::CommandInfo> GroupCmdParser::getCmdInfoList()
{
    using namespace std::placeholders;
    std::vector<CommandInfo> cmdInfoList = CmdParser::getCmdInfoList();
    cmdInfoList.insert(cmdInfoList.end(), {
        {
            Cmd::ListFriend,
            CommandInfo::Performer::Admin,
            std::bind(&GroupCmdParser::onListFriend, this, _1, _2, _3, _4),
            "[" + Cmd::ListFriend + "] List all friends."
        }, {
            Cmd::InviteFriend,
            CommandInfo::Performer::Admin,
            std::bind(&GroupCmdParser::onInviteFriend, this, _1, _2, _3, _4),
            "[" + Cmd::InviteFriend + " address (brief)] Invite a friend into group."
        }, {
            Cmd::KickFriend,
            CommandInfo::Performer::Admin,
            std::bind(&GroupCmdParser::onKickFriend, this, _1, _2, _3, _4),
            "[" + Cmd::KickFriend + " id] Kick a friend from group."
        }, {
            Cmd::ForwardMessage,
            CommandInfo::Performer::Member,
            std::bind(&GroupCmdParser::onForwardMessage, this, _1, _2, _3, _4),
            "[" + Cmd::ForwardMessage + "] Forward message to all online peer."
        },
    });

    return cmdInfoList;
}

/* =========================================== */
/* === class private function implement  ===== */
/* =========================================== */
int GroupCmdParser::onListFriend(const std::weak_ptr<Carrier>& carrier,
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

int GroupCmdParser::onInviteFriend(const std::weak_ptr<Carrier>& carrier,
                                   const std::vector<std::string>& args,
                                   const std::string& controller, int64_t timestamp)
{
    auto carrierPtr = carrier.lock();
    CHECK_ASSERT(carrierPtr, ErrCode::PointerReleasedError);

    if(args.size() < 1) {
        carrierPtr->sendMessage(controller, PromptBadArguments);
        CHECK_ERROR(ErrCode::InvalidArgument);
    }
    const auto& address = args[0];
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

    rc = getStorage()->updateMember(friendId, 0 /*timestamp*/, friendName, Storage::MemberStatus::Member);
    CHECK_ERROR(rc);

    return 0;
}

int GroupCmdParser::onKickFriend(const std::weak_ptr<Carrier>& carrier,
                                 const std::vector<std::string>& args,
                                 const std::string& controller, int64_t timestamp)
{
    auto carrierPtr = carrier.lock();
    CHECK_ASSERT(carrierPtr, ErrCode::PointerReleasedError);

    if(args.size() < 1) {
        carrierPtr->sendMessage(controller, PromptBadArguments);
        CHECK_ERROR(ErrCode::InvalidArgument);
    }
    const auto& memberId = args[0];

    carrierPtr->sendMessage(memberId, PromptKicked); // ignore return value

    int rc = carrierPtr->removeFriend(memberId);
    CHECK_ERROR(rc);

    rc = getStorage()->updateMember(memberId, Storage::MemberStatus::Blocked);
    CHECK_ERROR(rc);

    return 0;
}

int GroupCmdParser::onForwardMessage(const std::weak_ptr<Carrier>& carrier,
                                     const std::vector<std::string>& args,
                                     const std::string& controller, int64_t timestamp)
{
    if(args.size() == 1) {
        const std::string& message = args[0];
        int rc = getStorage()->updateMessage({timestamp, controller, message});
        CHECK_ERROR(rc);
    }

    int rc = forwardMsgToAllFriends(carrier);
    CHECK_ERROR(rc);

    return 0;
}

int GroupCmdParser::forwardMsgToAllFriends(const std::weak_ptr<Carrier>& carrier)
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

int GroupCmdParser::forwardMsgToFriend(const std::weak_ptr<Carrier>& carrier,
                                       const std::string& friendId)
{
    auto carrierPtr = carrier.lock();
    CHECK_ASSERT(carrierPtr, ErrCode::PointerReleasedError);

    int found;
    int64_t uptime = getStorage()->uptime(friendId);
    CHECK_ERROR(uptime);
    constexpr const int reservedId = 5;
    constexpr const int count = 10;

    do {
        std::vector<Storage::MessageInfo> msgList;
        found = getStorage()->findMessages(uptime, 10, friendId, msgList);
        CHECK_ASSERT(found >= 0 || found == ErrCode::NotFoundError, found);

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

    int rc = getStorage()->updateMember(friendId, uptime);
    CHECK_ERROR(rc);

    return 0;
}

} // namespace elastos