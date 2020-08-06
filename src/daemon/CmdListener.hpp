/**
 * @file	CarrierHandler.hpp
 * @brief	CarrierHandler
 * @details	
 *
 * @author	xxx
 * @author	<xxx@xxx.com>
 * @copyright	(c) 2012 xxx All rights reserved.
 **/

#ifndef _ELASTOS_CMD_LISTENER_HPP_
#define _ELASTOS_CMD_LISTENER_HPP_

#include <vector>
#include <CarrierHandler.hpp>

namespace elastos {

class Carrier;
class CmdParser;

class CmdListener : public CarrierHandler {
public:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/
    explicit CmdListener() = default;;
    virtual ~CmdListener() = default;

    int config(std::weak_ptr<Carrier> carrier);

    virtual void onError(int errCode) override;

    virtual void onStatusChanged(const std::string& userId,
                                 Status status) override;

    virtual void onFriendRequest(const std::string& friendCode,
                                 const std::string& summary) override;

    virtual void onFriendStatusChanged(const std::string& friendCode,
                                       Status status) override;

    virtual void onReceivedMessage(const std::string& friendCode,
                                   int64_t timestamp,
                                   const std::vector<uint8_t>& message) override;

protected:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/

private:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/
    std::weak_ptr<Carrier> carrier;
    std::shared_ptr<CmdParser> cmdParser;
}; // class CmdListener

} // namespace elastos

#endif /* _ELASTOS_CMD_LISTENER_HPP_ */