/**
 * @file	CarrierHandler.hpp
 * @brief	CarrierHandler
 * @details	
 *
 * @author	xxx
 * @author	<xxx@xxx.com>
 * @copyright	(c) 2012 xxx All rights reserved.
 **/

#ifndef _ELASTOS_DAEMON_HPP_
#define _ELASTOS_DAEMON_HPP_

#include <getopt.h>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace elastos {

class Carrier;

class Daemon {
public:
    /*** type define ***/

    /*** static function and variable ***/
    static std::shared_ptr<Daemon> GetInstance();

    /*** class function and variable ***/
    int run();

protected:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/

private:
    /*** type define ***/

    /*** static function and variable ***/
    static std::recursive_mutex DaemonMutex;
    static std::shared_ptr<Daemon> DaemonInstance;

    /*** class function and variable ***/
    explicit Daemon() = default;
    virtual ~Daemon() = default;

    std::shared_ptr<Carrier> carrier;

}; // class Daemon

} // namespace elastos

#endif /* _ELASTOS_DAEMON_HPP_ */