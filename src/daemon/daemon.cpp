#include <thread>

#include <Carrier.hpp>
#include <ErrCode.hpp>
#include <Log.hpp>
#include <Json.hpp>

int main(int argc, char **argv)
{
    int rc;

    auto options = std::make_shared<elastos::CarrierOptions>();
    options->logLevel = 4;
    options->enableUdp = true;
    options->persistentLocation = ".";
    options->bootstrapNodes.push_back({ "13.58.208.50",  "33445", "89vny8MrKdDKs7Uta9RdVmspPjnRMdwMmaiEW27pZ7gh" });
    options->bootstrapNodes.push_back({ "18.216.6.197",  "33445", "H8sqhRrQuJZ6iLtP2wanxt4LzdNrN2NNFnpPdq1uJ9n2" });
    options->bootstrapNodes.push_back({ "18.216.102.47", "33445", "G5z8MqiNDFTadFUPfMdYsYtkUDbX5mNCMVHMZtsCnFeb" });
    options->bootstrapNodes.push_back({ "52.83.127.85",  "33445", "CDkze7mJpSuFAUq6byoLmteyGYMeJ6taXxWoVvDMexWC" });
    options->bootstrapNodes.push_back({ "52.83.127.216", "33445", "4sL3ZEriqW7pdoqHSoYXfkc1NMNpiMz7irHMMrMjp9CM" });
    options->bootstrapNodes.push_back({ "52.83.171.135", "33445", "5tuHgK1Q4CYf4K5PutsEPK5E3Z7cbtEBdx7LwmdzqXHL" });
    options->expressNode.push_back({ "ece00.trinity-tech.io", "443", "FyTt6cgnoN1eAMfmTRJCaX2UoN6ojAgCimQEbv1bruy9" });
    options->expressNode.push_back({ "ece01.trinity-tech.io", "443", "FyTt6cgnoN1eAMfmTRJCaX2UoN6ojAgCimQEbv1bruy9" });
    options->expressNode.push_back({ "ece01.trinity-tech.cn", "443", "FyTt6cgnoN1eAMfmTRJCaX2UoN6ojAgCimQEbv1bruy9" });

    auto carrier = std::make_shared<elastos::Carrier>();
    
    rc = carrier->config(options, nullptr);
    CHECK_ERROR(rc);

    rc = carrier->start();
    CHECK_ERROR(rc);

    std::string address;
    rc = carrier->getAddress(address);
    CHECK_ERROR(rc);
    Log::W(Log::TAG, "Carrier started. address: %s", address.c_str());

    while(true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
