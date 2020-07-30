/**
 * @file	CARRIER.hpp
 * @brief	CARRIER
 * @details	
 *
 * @author	xxx
 * @author	<xxx@xxx.com>
 * @copyright	(c) 2012 xxx All rights reserved.
 **/

#ifndef _ELASTOS_CARRIER_HPP_
#define _ELASTOS_CARRIER_HPP_

#include <memory>

#include <ela_carrier.h>
#include <CarrierHandler.hpp>
#include <CarrierOptions.hpp>
#include <ThreadPool.hpp>

namespace elastos {

class Carrier : public std::enable_shared_from_this<Carrier> {
public:
    /*** type define ***/

    /*** static function and variable ***/
    static int GetUsrIdByAddress(const std::string& address, std::string& usrId);

    /*** class function and variable ***/
    explicit Carrier() = default;
    virtual ~Carrier() = default;

    virtual int config(std::shared_ptr<CarrierOptions> options,
                       std::shared_ptr<CarrierHandler> listener);

    virtual int start();

    virtual int getAddress(std::string& address);

    virtual bool isReady();

    virtual int requestFriend(const std::string& friendCode,
                              const std::string& summary = "");

    virtual int removeFriend(const std::string& friendCode);

    virtual int sendMessage(const std::string& friendCode,
                            const std::vector<uint8_t>& message);
    virtual int sendMessage(const std::string& friendCode,
                            const std::string& message);

    virtual int getFriendNameById(const std::string& id, std::string& name);

private:
    /*** type define ***/

    /*** static function and variable ***/
    static void GetElaCarrierError(int& errCode, std::string& errStr);
    static void OnCarrierConnection(ElaCarrier *carrier,
                                    ElaConnectionStatus status, void *context);
    static void OnCarrierFriendRequest(ElaCarrier *carrier,
                                       const char *friendid, const ElaUserInfo *info,
                                       const char *hello, void *context);
    static void OnCarrierFriendConnection(ElaCarrier *carrier,const char *friendid,
                                          ElaConnectionStatus status, void *context);
    static void OnCarrierFriendMessage(ElaCarrier *carrier, const char *from,
                                       const void *msg, size_t len,
                                       int64_t timestamp, bool offline, void *context);

    /*** class function and variable ***/
    void runCarrier();

    std::shared_ptr<CarrierHandler> listener;
    std::unique_ptr<ThreadPool> taskThread;
    std::shared_ptr<ElaCarrier> elaCarrierImpl;
    CarrierHandler::Status status;

}; // class Carrier

} // namespace elastos

#endif /* _ELASTOS_CARRIER_HPP_ */
