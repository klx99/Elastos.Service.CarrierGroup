/**
 * @file	Process.hpp
 * @brief	Process
 * @details	
 *
 * @author	xxx
 * @author	<xxx@xxx.com>
 * @copyright	(c) 2012 xxx All rights reserved.
 **/

#ifndef _ELASTOS_PROCESS_HPP_
#define _ELASTOS_PROCESS_HPP_

#include <string>
#include <vector>

namespace elastos {

class Process final {
public:
    /*** type define ***/

    /*** static function and variable ***/
    static int Exec(const std::string& execPath,
                    const std::vector<std::string>& execArgs);

    /*** class function and variable ***/


protected:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/
    explicit Process() = delete;
    virtual ~Process() = delete;

}; // class Process

} // namespace elastos

#endif // _ELASTOS_PROCESS_HPP_