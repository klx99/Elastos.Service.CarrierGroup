#include "CmdListener.hpp"

#include <vector>
#include <Carrier.hpp>
#include <CmdParser.hpp>
#include <DateTime.hpp>
#include <ErrCode.hpp>
#include <GroupCmdParser.hpp>
#include <Log.hpp>
#include <OptParser.hpp>

namespace elastos {

/* =========================================== */
/* === static variables initialize =========== */
/* =========================================== */

/* =========================================== */
/* === static function implement ============= */
/* =========================================== */

/* =========================================== */
/* === class public function implement  ====== */
/* =========================================== */
int CmdListener::config(std::weak_ptr<Carrier> carrier)
{
    this->carrier = carrier;
    this->cmdParser = CmdParser::Factory::Create();

    int rc = this->cmdParser->config(carrier);
    CHECK_ERROR(rc);

    return 0;
}

void CmdListener::onError(int errCode)
{
    Log::D(Log::TAG, "%s errCode=%d", __PRETTY_FUNCTION__, errCode);
}

void CmdListener::onStatusChanged(const std::string& userId,
                                  Status status)
{
    Log::D(Log::TAG, "%s", __PRETTY_FUNCTION__);
}

void CmdListener::onFriendRequest(const std::string& friendCode,
                                  const std::string& summary)
{
    Log::D(Log::TAG, "%s", __PRETTY_FUNCTION__);
    auto cmdline = CmdParser::Cmd::AddFriend + " " + friendCode + " " + summary;
    int rc = cmdParser->parse(carrier, 
                              cmdline, friendCode, 0);
    if(rc < 0) {
        onError(rc);
        return;
    }

    return;
}

void CmdListener::onFriendStatusChanged(const std::string& friendCode,
                                        Status status)
{
    Log::D(Log::TAG, "%s", __PRETTY_FUNCTION__);

    bool isGroup = !OptParser::GetInstance()->isManager();
    if(isGroup && status == Status::Online) {
        int rc = cmdParser->parse(carrier,
                                  GroupCmdParser::Cmd::ForwardMessage,
                                  friendCode, DateTime::CurrentNS());
        CHECK_RETVAL(rc);
    }
}

void CmdListener::onReceivedMessage(const std::string& friendCode,
                                    int64_t timestamp,
                                    const std::vector<uint8_t>& message)
{
    Log::D(Log::TAG, "%s", __PRETTY_FUNCTION__);

    std::string cmdline = std::string{message.begin(), message.end()};

    // ignore hyper return receipt
    if(cmdline == "{\"command\":\"messageSeen\"}") {
        return;
    }

    if(cmdline.find_first_of('/') != 0) { // if not a command, exec as forward.
        cmdline = GroupCmdParser::Cmd::ForwardMessage + " " + cmdline;
    }
    int rc = cmdParser->parse(carrier,
                              cmdline, friendCode, timestamp);
    CHECK_RETVAL(rc);
}

/* =========================================== */
/* === class protected function implement  === */
/* =========================================== */


/* =========================================== */
/* === class private function implement  ===== */
/* =========================================== */


} // namespace elastos
