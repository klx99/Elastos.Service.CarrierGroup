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
            "[" + Cmd::NewGroup + " groupname] New a new group."
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

    if(args.size() < 1) {
        carrierPtr->sendMessage(controller, PromptBadArguments);
        CHECK_ERROR(ErrCode::InvalidArgument);
    }
    const auto& groupName = args[0];
    
    auto groupDataDir = std::filesystem::canonical(OptParser::GetInstance()->getDataDir());
    groupDataDir = groupDataDir / controller /DateTime::NsToString(timestamp, false);
    std::vector<std::string> groupArgs = {
        "--group",
        std::string("--") + OptParser::OptName::DataDir + "=" + groupDataDir.string(),
    };

    int rc = Process::Exec(OptParser::GetInstance()->getExecPath(), groupArgs);
    CHECK_ERROR(rc);

    std::string newGroupAddress;
    constexpr const int tick = 100; // milliseconds
    for(auto idx = 0; idx < (MaxWaitNewGroupTime / tick); idx++) {
        auto addrFilePath = std::filesystem::path {groupDataDir};
        std::ifstream addrFile {addrFilePath / CarrierAddressName};
        addrFile >> newGroupAddress;
        if(newGroupAddress.empty() == false) {
            break;
        }

        Log::D(Log::TAG, "Waiting new group start...");
        getTaskThread()->sleepMS(tick);
    }

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

} // namespace elastos