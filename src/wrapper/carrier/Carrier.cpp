//
//  Carrier.cpp
//
//  Created by mengxk on 19/03/16.
//  Copyright © 2016 mengxk. All rights reserved.
//

#include "Carrier.hpp"

#include <functional>
#include <sstream>

#include <ela_carrier.h>
#include <ErrCode.hpp>
#include <Log.hpp>

namespace elastos {

/***********************************************/
/***** static variables initialize *************/
/***********************************************/


/***********************************************/
/***** static function implement ***************/
/***********************************************/
bool Carrier::CheckAddress(const std::string& address)
{
    bool valid = ela_address_is_valid(address.c_str());
    return valid;
}

int Carrier::GetUsrIdByAddress(const std::string& address, std::string& usrId)
{
    char buf[ELA_MAX_ID_LEN + 1] = {0};
    auto userId = ela_get_id_by_address(address.c_str(), buf, sizeof(buf));
    if(userId == nullptr) {
        int elaErrCode;
        std::string elaErrStr;
        GetElaCarrierError(elaErrCode, elaErrStr);
        Log::E(Log::TAG, "Failed to get carrier usr id from:%s! rc=%s(0x%x)",
                         address.c_str(), elaErrStr.c_str(), elaErrCode);
    }
    CHECK_ASSERT(userId, ErrCode::CarrierError);

    usrId = buf;
    return 0;
}

/***********************************************/
/***** class public function implement  ********/
/***********************************************/
int Carrier::config(std::shared_ptr<CarrierOptions> options,
                    std::shared_ptr<CarrierHandler> listener)
{
    int rc;

    CHECK_ASSERT(options, ErrCode::CarrierBadOptions);
    CHECK_ASSERT(elaCarrierImpl == nullptr, ErrCode::CarrierMultiConfigError);

    this->listener = listener;

    ElaOptions elaOptions;
    ElaCallbacks elaCallbacks;
    memset(&elaCallbacks, 0, sizeof(elaCallbacks));
    memset(&elaOptions, 0, sizeof(elaOptions));

    elaOptions.log_level = static_cast<ElaLogLevel>(options->logLevel);
    elaOptions.udp_enabled = options->enableUdp;
    elaOptions.persistent_location = options->persistentLocation.c_str();

    size_t bootNodeSize = options->bootstrapNodes.size();
    BootstrapNode bootNodeArray[bootNodeSize];
    memset (bootNodeArray, 0, sizeof(bootNodeArray));
    for(int idx = 0; idx < bootNodeSize; idx++) {
        const auto& node = options->bootstrapNodes[idx];
        bootNodeArray[idx].ipv4 = node.ipv4.c_str();
        bootNodeArray[idx].port = node.port.c_str();
        bootNodeArray[idx].public_key = node.publicKey.c_str();
    }
    elaOptions.bootstraps_size = bootNodeSize;
    elaOptions.bootstraps = bootNodeArray;

    size_t expressNodeSize = options->expressNode.size();
    ExpressNode expressNodeArray[expressNodeSize];
    memset (expressNodeArray, 0, sizeof(expressNodeArray));
    for(int idx = 0; idx < expressNodeSize; idx++) {
        const auto& node = options->expressNode[idx];
        expressNodeArray[idx].ipv4 = node.ipv4.c_str();
        expressNodeArray[idx].port = node.port.c_str();
        expressNodeArray[idx].public_key = node.publicKey.c_str();
    }
    elaOptions.express_nodes_size = expressNodeSize;
    elaOptions.express_nodes = expressNodeArray;

    elaCallbacks.connection_status = OnCarrierConnection;
    elaCallbacks.friend_request = OnCarrierFriendRequest;
    elaCallbacks.friend_connection = OnCarrierFriendConnection;
    elaCallbacks.friend_message = OnCarrierFriendMessage;

    auto creater = [&]() -> ElaCarrier* {
        Log::I(Log::TAG, "Carrier::start() new ela carrier");
        auto ptr = ela_new(&elaOptions, &elaCallbacks, this);
        return ptr;
    };
    auto deleter = [=](ElaCarrier* ptr) -> void {
        Log::I(Log::TAG, "Carrier::start() kill ela carrier");
        if(ptr != nullptr) {
            ela_kill(ptr);
        }
    };
    elaCarrierImpl = std::shared_ptr<ElaCarrier>(creater(), deleter);
    if (elaCarrierImpl == nullptr) {
        int elaErrCode;
        std::string elaErrStr;
        GetElaCarrierError(elaErrCode, elaErrStr);
        Log::E(Log::TAG, "Failed to new carrier! carrier error is %s(0x%x)",
                         elaErrStr.c_str(), elaErrCode);
    }
    CHECK_ASSERT(elaCarrierImpl, ErrCode::CarrierCreateFailed);

    return 0;
}

int Carrier::start()
{
    if(taskThread == nullptr) {
        taskThread = std::make_unique<ThreadPool>("carrier-thread");
    }
    taskThread->post(std::bind(&Carrier::runCarrier, this));

    return 0;
}

int Carrier::getAddress(std::string& address)
{
    char buf[ELA_MAX_ADDRESS_LEN + 1] = {0};
    auto addr = ela_get_address(elaCarrierImpl.get(), buf, sizeof(buf));
    if(addr == nullptr) {
        int elaErrCode;
        std::string elaErrStr;
        GetElaCarrierError(elaErrCode, elaErrStr);
        Log::E(Log::TAG, "Failed to get address! rc=%s(0x%x)", elaErrStr.c_str(), elaErrCode);
    }
    CHECK_ASSERT(addr, ErrCode::CarrierError);

    address = addr;
    return 0;
}

bool Carrier::isReady()
{
    return (status == CarrierHandler::Status::Online);
}

int Carrier::addFriend(const std::string& friendCode,
                       const std::string& summary)
{
    std::string selfAddr, selfUsrId;
    getAddress(selfAddr);
    GetUsrIdByAddress(selfAddr, selfUsrId);
    CHECK_ASSERT(friendCode != selfAddr && friendCode != selfUsrId,
                 ErrCode::CarrierFriendSelf);

    bool isAddr = ela_address_is_valid(friendCode.c_str());
    bool isUserId = ela_id_is_valid(friendCode.c_str());
    CHECK_ASSERT(isAddr || isUserId, ErrCode::InvalidArgument);

    int rc = 0;
    if(isAddr == true) {
        const char* hello = (summary.empty() ? " " : summary.c_str());

        Log::I(Log::TAG, "Carrier::addFriend() friendCode=%s summary=%s", friendCode.c_str(), hello);
        rc = ela_add_friend(elaCarrierImpl.get(), friendCode.c_str(), hello);
    } else if(isUserId == true) {
        Log::I(Log::TAG, "Carrier::addFriend() friendCode=%s", friendCode.c_str());
        rc = ela_accept_friend(elaCarrierImpl.get(), friendCode.c_str());
    }
    if(rc != 0) {
        int elaErrCode;
        std::string elaErrStr;
        GetElaCarrierError(elaErrCode, elaErrStr);
        if(elaErrCode == ELA_GENERAL_ERROR(ELAERR_ALREADY_EXIST)) {
            return ErrCode::CarrierFriendExists;
        }
        Log::E(Log::TAG, "Failed to add/accept friend! rc=%s(0x%x)", elaErrStr.c_str(), elaErrCode);
    }
    CHECK_ASSERT(rc == 0, ErrCode::CarrierAddFriendFailed);

    return 0;
}

int Carrier::removeFriend(const std::string& friendCode)
{
    std::string usrId;
    bool valid = ela_id_is_valid(friendCode.c_str());
    if(valid == true) {
        usrId = friendCode;
    } else {
        bool valid = ela_address_is_valid(friendCode.c_str());
        if (valid == true) {
            GetUsrIdByAddress(friendCode, usrId);
        }
    }
    CHECK_ASSERT(!usrId.empty(), ErrCode::InvalidArgument);

    bool isFriend = ela_is_friend(elaCarrierImpl.get(), usrId.c_str());
    if(isFriend == false) {
        Log::W(Log::TAG, "Ignore to remove a friend: %s, it's not exists.", usrId.c_str());
        return 0;
    }

    int rc = ela_remove_friend(elaCarrierImpl.get(), usrId.c_str());
    if(rc != 0) {
        int elaErrCode;
        std::string elaErrStr;
        GetElaCarrierError(elaErrCode, elaErrStr);
        Log::E(Log::TAG, "Failed to remove friend! rc=%s(0x%x)", elaErrStr.c_str(), elaErrCode);
    }
    CHECK_ASSERT(rc == 0, ErrCode::CarrierRemoveFriendFailed);

    return 0;
}

int Carrier::sendMessage(const std::string& friendCode,
                         const std::vector<uint8_t>& message)
{
    bool offlineMsg;
    int rc = ela_send_friend_message(elaCarrierImpl.get(), friendCode.c_str(), message.data(), message.size(), &offlineMsg);
    if(rc != 0) {
        int elaErrCode;
        std::string elaErrStr;
        GetElaCarrierError(elaErrCode, elaErrStr);
        Log::E(Log::TAG, "Failed to send message! rc=%s(0x%x)", elaErrStr.c_str(), elaErrCode);
    }
    CHECK_ASSERT(rc == 0, ErrCode::CarrierRemoveFriendFailed);

    Log::D(Log::TAG, "Carrier::sendMessage() success to send %d bytes.", message.size());
    return 0;
}

int Carrier::sendMessage(const std::string& friendCode,
                         const std::string& message)
{
    std::vector<uint8_t> buf {message.begin(), message.end()};

    int rc = sendMessage(friendCode, buf);
    CHECK_ERROR(rc);

    return rc;
}

int Carrier::sendMessage(const std::string& friendCode,
                         const std::string_view& message)
{
    std::vector<uint8_t> buf {message.begin(), message.end()};

    int rc = sendMessage(friendCode, buf);
    CHECK_ERROR(rc);

    return rc;
}

int Carrier::getFriendNameById(const std::string& id, std::string& name)
{
    ElaFriendInfo info;
    int rc = ela_get_friend_info(elaCarrierImpl.get(), id.c_str(), &info);
    if(rc < 0) {
        int elaErrCode;
        std::string elaErrStr;
        GetElaCarrierError(elaErrCode, elaErrStr);
        Log::E(Log::TAG, "Failed to get carrier friend name from:%s! rc=%s(0x%x)",
                         id.c_str(), elaErrStr.c_str(), elaErrCode);
    }
    CHECK_ASSERT(rc == 0, ErrCode::CarrierError);

    name = info.user_info.name;

    return 0;
}

int Carrier::getFriendList(bool onlineOnly, std::vector<std::string>& list)
{
    list.clear();
    auto callback = std::make_shared<std::function<void(const char*, ElaConnectionStatus)>>([&](const char* friendId, ElaConnectionStatus status) {
        if(onlineOnly == true
        && status != ElaConnectionStatus_Connected) {
            return;
        }

        list.push_back(friendId);
    });

    int rc = ela_get_friends(elaCarrierImpl.get(), ElaFriendsIterateCallback, callback.get());

    if(rc < 0) {
        list.clear();

        int elaErrCode;
        std::string elaErrStr;
        GetElaCarrierError(elaErrCode, elaErrStr);
        Log::E(Log::TAG, "Failed to get carrier friend list! rc=%s(0x%x)",
                         elaErrStr.c_str(), elaErrCode);
    }
    CHECK_ASSERT(rc == 0, ErrCode::CarrierError);

    return 0;
}


/***********************************************/
/***** class protected function implement  *****/
/***********************************************/


/***********************************************/
/***** class private function implement  *******/
/***********************************************/
bool Carrier::ElaFriendsIterateCallback(const ElaFriendInfo *info, void *context)
{
    if(info == nullptr) {
        return 0;
    }

    auto callback = reinterpret_cast<std::function<void(const char*, ElaConnectionStatus)>*>(context);
    (*callback)(info->user_info.userid, info->status);

    return true;
}

void Carrier::GetElaCarrierError(int& errCode, std::string& errStr)
{
    errCode = ela_get_error();

    char buf[1024] = {0};
    ela_get_strerror(errCode, buf, sizeof(buf));
    errStr = buf;
}

void Carrier::OnCarrierConnection(ElaCarrier *carrier,
                                  ElaConnectionStatus status, void *context)
{
    auto thiz = reinterpret_cast<Carrier*>(context);

    std::string address, userId;
    thiz->getAddress(address);
    int rc = GetUsrIdByAddress(address, userId);

    thiz->status = (status == ElaConnectionStatus_Connected
                            ? CarrierHandler::Status::Online
                            : CarrierHandler::Status::Offline);
    Log::D(Log::TAG, "Carrier::OnCarrierConnection() status: %d", status);

    if(thiz->listener.get() != nullptr) {
        if(rc < 0) {
            thiz->listener->onError(rc);
        } else {
            thiz->listener->onStatusChanged(userId, thiz->status);
        }
    }
}

void Carrier::OnCarrierFriendRequest(ElaCarrier *carrier, const char *friendid,
                                     const ElaUserInfo *info,
                                     const char *hello, void *context)
{
    Log::D(Log::TAG, "Carrier::OnCarrierFriendRequest() from: %s", friendid);
    auto thiz = reinterpret_cast<Carrier*>(context);

    if(thiz->listener.get() != nullptr) {
        thiz->listener->onFriendRequest(friendid, hello);
    }
}

void Carrier::OnCarrierFriendConnection(ElaCarrier *carrier,const char *friendid,
                                        ElaConnectionStatus status, void *context)
{
    auto thiz = reinterpret_cast<Carrier*>(context);

    auto frdStatus = (status == ElaConnectionStatus_Connected
                             ? CarrierHandler::Status::Online
                             : CarrierHandler::Status::Offline);
    Log::D(Log::TAG, "Carrier::OnCarrierFriendConnection() from: %s %d", friendid, frdStatus);

    if(thiz->listener.get() != nullptr) {
        thiz->listener->onFriendStatusChanged(friendid, frdStatus);
    }
}

void Carrier::OnCarrierFriendMessage(ElaCarrier *carrier, const char *from,
                                     const void *msg, size_t len,
                                     int64_t timestamp, bool offline, void *context)
{
    Log::D(Log::TAG, "Carrier::OnCarrierFriendMessage() from: %s len=%d", from, len);

    auto thiz = reinterpret_cast<Carrier*>(context);
    if(thiz->listener == nullptr) {
        Log::W(Log::TAG, "ChannelListener is not set. ignore to process received message.");
        return;
    }

    auto data = reinterpret_cast<const uint8_t*>(msg);
    auto message = std::vector<uint8_t>(data, data + len);
    thiz->listener->onReceivedMessage(from, timestamp, message);
}

void Carrier::runCarrier()
{
    int rc= ela_run(elaCarrierImpl.get(), 500);
    if(rc < 0) {
        int elaErrCode;
        std::string elaErrStr;
        GetElaCarrierError(elaErrCode, elaErrStr);
        Log::E(Log::TAG, "Failed to run ela carrier. rc=%s(0x%x)", elaErrStr.c_str(), elaErrCode);

        if (this->listener.get() != nullptr) {
            this->listener->onError(rc);
        }
    }

    return;
}

} // namespace elastos
