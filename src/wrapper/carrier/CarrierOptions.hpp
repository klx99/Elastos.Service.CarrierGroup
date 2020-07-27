/**
 * @file	CarrierOptions.hpp
 * @brief	CarrierOptions
 * @details	
 *
 * @author	xxx
 * @author	<xxx@xxx.com>
 * @copyright	(c) 2012 xxx All rights reserved.
 **/

#ifndef _ELASTOS_CARRIER_OPTIONS_HPP_
#define _ELASTOS_CARRIER_OPTIONS_HPP_

#include <string>
#include <vector>

namespace elastos {

struct CarrierOptions {
public:
    /*** type define ***/
    struct Node {
        const std::string ipv4;
        const std::string port;
    };
    struct BootstrapNode : Node {
        const std::string publicKey;
    };
    struct ExpressNode : Node {
        const std::string publicKey;
    };

    /*** static function and variable ***/

    /*** class function and variable ***/
    explicit CarrierOptions() = default;
    virtual ~CarrierOptions() = default;

    int logLevel;
    std::string persistentLocation;
    bool enableUdp;
    std::vector<BootstrapNode> bootstrapNodes;
    std::vector<ExpressNode> expressNode;

private:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/

}; // class CarrierOptions

} // namespace elastos

#endif /* _ELASTOS_CARRIER_OPTIONS_HPP_ */