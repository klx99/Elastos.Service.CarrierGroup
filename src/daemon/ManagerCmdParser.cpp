#include "ManagerCmdParser.hpp"

#include <string>
#include <sstream>
#include <vector>
#include <Carrier.hpp>
#include <CompatibleFileSystem.hpp>
#include <DateTime.hpp>
#include <ErrCode.hpp>
#include <Log.hpp>
#include <OptParser.hpp>
#include <Process.hpp>

namespace elastos {

/* =========================================== */
/* === static variables initialize =========== */
/* =========================================== */
const int ManagerCmdParser::MaxWaitNewGroupTime = 1000; // millisecond

/* =========================================== */
/* === static function implement ============= */
/* =========================================== */

/* =========================================== */
/* === class public function implement  ====== */
/* =========================================== */

/* =========================================== */
/* === class protected function implement  === */
/* =========================================== */
std::vector<CmdParser::CommandInfo> ManagerCmdParser::getCmdInfoList()
{
    using namespace std::placeholders;
    std::vector<CommandInfo> cmdInfoList = CmdParser::getCmdInfoList();
    cmdInfoList.insert(cmdInfoList.end(), {
        {
            Cmd::NewGroup,
            CommandInfo::Performer::Anyone,
            std::bind(&ManagerCmdParser::onNewGroup, this, _1, _2, _3, _4),
            "[" + Cmd::NewGroup + "] New a new group."
        }, {
            Cmd::StartGroup,
            CommandInfo::Performer::Anyone,
            std::bind(&ManagerCmdParser::onStartGroup, this, _1, _2, _3, _4),
            "[" + Cmd::StartGroup + " groupaddress] New a new group."
        }
    });

    return cmdInfoList;
}

/* =========================================== */
/* === class private function implement  ===== */
/* =========================================== */
int ManagerCmdParser::onNewGroup(const std::weak_ptr<Carrier>& carrier,
                                 const std::vector<std::string>& args,
                                 const std::string& controller, int64_t timestamp)
{
    auto carrierPtr = carrier.lock();
    CHECK_ASSERT(carrierPtr, ErrCode::PointerReleasedError);

    // if(args.size() < 1) {
    //     carrierPtr->sendMessage(controller, PromptBadArguments);
    //     CHECK_ERROR(ErrCode::InvalidArgument);
    // }
    // const auto& groupName = args[0];
    
    auto groupDataDir = std::filesystem::path(controller) / DateTime::NsToString(timestamp, false);
    auto groupDataPath = OptParser::GetInstance()->getDataDir() / groupDataDir;
    std::vector<std::string> groupArgs = {
        "--group",
        std::string("--") + OptParser::OptName::DataDir + "=" + groupDataPath.string(),
    };

    int rc = Process::Exec(OptParser::GetInstance()->getExecPath(), groupArgs);
    CHECK_ERROR(rc);

    std::string newGroupAddress;
    constexpr const int tick = 100; // milliseconds
    for(auto idx = 0; idx < (MaxWaitNewGroupTime / tick); idx++) {
        std::ifstream addrFile {groupDataPath / CarrierAddressName};
        addrFile >> newGroupAddress;
        if(newGroupAddress.empty() == false) {
            break;
        }

        Log::D(Log::TAG, "Waiting new group start...");
        getTaskThread()->sleepMS(tick);
    }

    rc = getStorage()->updateManager(timestamp, newGroupAddress, groupDataDir.string());
    CHECK_ERROR(rc);

    std::string response;
    if(newGroupAddress.empty() == true) {
        response = "Failed to new a group.";
    } else {
        response = "Success to new the group: " + newGroupAddress;
    }

    rc = carrierPtr->sendMessage(controller, response);
    CHECK_ERROR(rc);

    return 0;
}

int ManagerCmdParser::onStartGroup(const std::weak_ptr<Carrier>& carrier,
                                   const std::vector<std::string>& args,
                                   const std::string& controller, int64_t timestamp)
{
    auto carrierPtr = carrier.lock();
    CHECK_ASSERT(carrierPtr, ErrCode::PointerReleasedError);

    if(args.size() < 1) {
        carrierPtr->sendMessage(controller, PromptBadArguments);
        CHECK_ERROR(ErrCode::InvalidArgument);
    }
    const auto& groupAddr = args[0];
    
    std::string savedGroupDir;
    int rc = getStorage()->findGroup(groupAddr, savedGroupDir);
    CHECK_ERROR(rc);

    auto groupDataDir = std::filesystem::path(savedGroupDir);
    auto groupDataPath = OptParser::GetInstance()->getDataDir() / groupDataDir;
    std::vector<std::string> groupArgs = {
        "--group",
        std::string("--") + OptParser::OptName::DataDir + "=" + groupDataPath.string(),
    };

    rc = Process::Exec(OptParser::GetInstance()->getExecPath(), groupArgs);
    CHECK_ERROR(rc);

    std::string response = "Success to start group: " + groupAddr;
    rc = carrierPtr->sendMessage(controller, response);
    CHECK_ERROR(rc);

    return 0;
}

} // namespace elastos