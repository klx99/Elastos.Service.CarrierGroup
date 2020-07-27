#include "CmdListener.hpp"

#include <vector>
#include <Carrier.hpp>
#include <CmdParser.hpp>
#include <DateTime.hpp>
#include <ErrCode.hpp>
#include <Log.hpp>

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
CmdListener::CmdListener(std::weak_ptr<Carrier> carrier)
    : carrier(carrier)
{
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
    auto cmdline = CmdParser::Cmd::RequestFriend + " " + friendCode + " " + summary;
    std::string response;

    int rc = CmdParser::GetInstance()->parse(carrier, 
                                             cmdline, friendCode, DateTime::CurrentMS());
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

    if(status == Status::Online) {
        int rc = CmdParser::GetInstance()->parse(carrier,
                                                 CmdParser::Cmd::ForwardMessage,
                                                 friendCode, DateTime::CurrentMS());
        CHECK_RETVAL(rc);
    }
}

void CmdListener::onReceivedMessage(const std::string& friendCode,
                                    int64_t timestamp,
                                    const std::vector<uint8_t>& message)
{
    Log::D(Log::TAG, "%s", __PRETTY_FUNCTION__);

    std::string cmdline = {message.begin(), message.end()};

    int rc = CmdParser::GetInstance()->parse(carrier,
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
