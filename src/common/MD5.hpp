/**
 * @file	MD5.hpp
 * @brief	MD5
 * @details	
 *
 * @author	xxx
 * @author	<xxx@xxx.com>
 * @copyright	(c) 2012 xxx All rights reserved.
 **/

#ifndef _ELASTOS_MD5_HPP_
#define _ELASTOS_MD5_HPP_

#include <string>
#include <vector>
#include <CompatibleFileSystem.hpp>

namespace elastos {

class MD5 {
public:
    /*** type define ***/
    //using Task = std::bind<F, Args...>;

    /*** static function and variable ***/
    static std::string Get(const std::vector<uint8_t>& data);
    static std::string Get(const std::string& data);
    static std::string Get(const elastos::filesystem::path& datapath);

    static std::string MakeHexString(const std::vector<uint8_t>& data);

    /*** class function and variable ***/

    // post and copy
    // post and move

private:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/
    explicit MD5() = delete;
    virtual ~MD5() = delete;

}; // class MD5

} // namespace elastos

#endif /* _ELASTOS_MD5_HPP_ */
