/**
 * @file	CarrierHandler.hpp
 * @brief	CarrierHandler
 * @details	
 *
 * @author	xxx
 * @author	<xxx@xxx.com>
 * @copyright	(c) 2012 xxx All rights reserved.
 **/

#ifndef _ELASTOS_CARRIER_HANDLER_HPP_
#define _ELASTOS_CARRIER_HANDLER_HPP_

#include <string>

namespace elastos {

class CarrierHandler {
public:
    /*** type define ***/
    enum Status {
        Online        = 1,
        Offline       = 2,
    };

    /*** static function and variable ***/
    virtual void onError(int errCode) = 0;

    virtual void onStatusChanged(const std::string& userId,
                                 Status status) = 0;

    virtual void onFriendRequest(const std::string& friendCode,
                                    const std::string& summary) = 0;

    virtual void onFriendStatusChanged(const std::string& friendCode,
                                        Status status) = 0;

    virtual void onReceivedMessage(const std::string& friendCode,
                                   int64_t timestamp,
                                   const std::vector<uint8_t>& message) = 0;

    /*** class function and variable ***/

protected:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/
    explicit CarrierHandler() = default;
    virtual ~CarrierHandler() = default;

private:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/

}; // class CARRIER_HANDLER

} // namespace elastos

#endif /* _ELASTOS_CARRIER_HANDLER_HPP_ */