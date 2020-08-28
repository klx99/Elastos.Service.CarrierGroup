//
//  MD5.cpp
//
//  Created by mengxk on 19/03/16.
//  Copyright © 2016 mengxk. All rights reserved.
//

#include "Process.hpp"

#include <cerrno>
#include <unistd.h>
#include <ErrCode.hpp>
#include <Log.hpp>

namespace elastos {

/***********************************************/
/***** static variables initialize *************/
/***********************************************/


/***********************************************/
/***** static function implement ***************/
/***********************************************/
int Process::Exec(const std::string& execPath,
                  const std::vector<std::string>& execArgs)
{
    pid_t pid = fork();
    switch(pid) {
    case -1:
        // An error has occurred
        CHECK_ERROR(ErrCode::StdSystemErrorIndex - errno);
    case 0:
        {
            // This code is executed by the child process
            Log::W(Log::TAG, "Start to exec child process by [%s].", execPath.c_str());
            std::vector<const char*> args;
            if(execArgs.size() > 0 && execPath != execArgs[0]) {
                args.push_back(execPath.data());
            }
            for(const auto& it: execArgs) {
                args.push_back(it.data());
            }
            args.push_back(nullptr);
            int rc = execv(execPath.data(), const_cast<char**>(args.data()));
            if(rc < 0) {
                Log::E(Log::TAG, "Failed to exec %s: (%d)%s", execPath.c_str(), errno, strerror(errno));
            }
            exit(rc);
            break;
        }
    default:
        // This code is executed by the parent process
        Log::I(Log::TAG, "Success to create child process...");
        break;
    }

    return 0;
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