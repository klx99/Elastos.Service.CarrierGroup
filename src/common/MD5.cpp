//
//  MD5.cpp
//
//  Created by mengxk on 19/03/16.
//  Copyright © 2016 mengxk. All rights reserved.
//

#if 0
#include "MD5.hpp"

#include <cstring>
#include <iomanip>
#include <openssl/md5.h>
#include <sstream>
#include "Log.hpp"


namespace elastos {

/***********************************************/
/***** static variables initialize *************/
/***********************************************/


/***********************************************/
/***** static function implement ***************/
/***********************************************/
std::string MD5::Get(const std::vector<uint8_t>& data)
{
    MD5_CTX ctx;
    uint8_t outmd[16];

    memset(outmd,0,sizeof(outmd));
    MD5_Init(&ctx);
    MD5_Update(&ctx, data.data(), data.size());
    MD5_Final(outmd, &ctx);

    std::string md5 = MakeHexString(std::vector<uint8_t>(std::begin(outmd), std::end(outmd)));
    return md5;
}

std::string MD5::Get(const std::string& data)
{
    std::vector<uint8_t> dataBytes(data.begin(), data.end());

    return Get(dataBytes);
}

std::string MD5::Get(const elastos::filesystem::path& datapath)
{
    MD5_CTX ctx;
    uint8_t outmd[16];
    memset(outmd,0,sizeof(outmd));
    MD5_Init(&ctx);

    int64_t filesize = elastos::filesystem::file_size(datapath);
    constexpr const int bufsize = 1024;
    char buffer[bufsize];
    auto ifs = std::ifstream(datapath, std::ifstream::binary);
    while (filesize > 0) {
        int readsize = (bufsize < filesize ? bufsize : filesize);
        ifs.read(buffer, readsize);

        MD5_Update(&ctx, buffer, readsize);

        Log::V(Log::TAG, "%s %d", FORMAT_METHOD, __LINE__);
        filesize -= readsize;
    }
    MD5_Final(outmd, &ctx);

    std::string md5 = MakeHexString(std::vector<uint8_t>(std::begin(outmd), std::end(outmd)));
    Log::V(Log::TAG, "%s %d", FORMAT_METHOD, __LINE__);
    return md5;
}

std::string MD5::MakeHexString(const std::vector<uint8_t>& data)
{
    std::ostringstream ss;

    ss << std::hex << std::setfill('0');
    ss << std::uppercase;

    for(const auto it: data) {
        ss << std::setw(2) << static_cast<int>(it);
    }

    return ss.str();
}


/***********************************************/
/***** class public function implement  ********/
/***********************************************/

/***********************************************/
/***** class protected function implement  *****/
/***********************************************/


/***********************************************/
/***** class private function implement  *******/
/***********************************************/

} // namespace elastos

#endif // 0