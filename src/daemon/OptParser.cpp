#include "OptParser.hpp"

#include <iomanip>
#include <getopt.h>
#include <sstream>
#include <CompatibleFileSystem.hpp>
#include <ErrCode.hpp>
#include <Log.hpp>

namespace elastos {

/* =========================================== */
/* === static variables initialize =========== */
/* =========================================== */
std::shared_ptr<OptParser> OptParser::OptParserInstance;
std::recursive_mutex OptParser::CmdMutex = {};

const int OptParser::MaxVisibleOpt = 0xFF;
const char* OptParser::ShortOptions = "hd:gm";
struct OptParser::Option OptParser::LongOptions[] = {
    //   NAME       ARGUMENT           FLAG  SHORTNAME
    {
        {OptName::Help,    no_argument,       nullptr, 'h'},
        "Print usage.",
    }, {
        {OptName::DataDir, required_argument, nullptr, 'd'},
        "Data location, override the option in config.",
    }, {
        {OptName::Manager, no_argument,       nullptr, MaxVisibleOpt + 1},
        "Run manager mode.",
    }, {
        {OptName::Group,   no_argument,       nullptr, MaxVisibleOpt + 2},
        "Run group mode.",
    }, {
        {nullptr,   0,                 nullptr, 0},
        nullptr,
    },
};


/* =========================================== */
/* === static function implement ============= */
/* =========================================== */
std::shared_ptr<OptParser> OptParser::GetInstance()
{
    if(OptParserInstance != nullptr) {
        return OptParserInstance;
    }

    std::lock_guard<std::recursive_mutex> lg(CmdMutex);
    if(OptParserInstance != nullptr) {
        return OptParserInstance;
    }

    struct Impl: OptParser {
    };
    OptParserInstance = std::make_shared<Impl>();

    return OptParserInstance;
}

/* =========================================== */
/* === class public function implement  ====== */
/* =========================================== */
int OptParser::parse(int argc, char **argv)
{

    execPath = argv[0];
    std::vector<char*> execArgs;
    for(auto idx = 1; idx < argc; idx++) { // argv[0] is exec path
        execArgs.push_back(argv[idx]);
    }

    Log::D(Log::TAG, "exec path: %s", execPath.c_str());
    for(auto idx = 0; idx < execArgs.size(); idx++) {
        Log::D(Log::TAG, "exec arg%d: %s", idx, execArgs[idx]);
    }

    bool groupFlag = false;
    int opt, optIdx = 0;
    std::vector<struct option> longOpts;
    for(const auto& it: LongOptions) {
        longOpts.push_back(it.opt);
    }
    while ((opt = getopt_long(argc, argv, ShortOptions, longOpts.data(), &optIdx)) != -1) {
        int thisOptInd = optind ? optind : 1;
        switch (opt) {
        case 'h':
            printUsage();
            exit(0);
            break;
        case 'd':
            dataDir = optarg;
            break;
        case MaxVisibleOpt + 1:
            managerFlag = true;
            break;
        case MaxVisibleOpt + 2:
            groupFlag = true;
            break;
        case '?':
            break;
        default:
            printf ("?? getopt returned character code 0x%X ??\n", opt);
        }
    }
    // if (optind < argc) {
    //     printf ("non-option ARGV-elements: ");
    //     while (optind < argc)
    //         printf ("%s ", argv[optind++]);
    //     printf ("\n");
    // }

    // Check args
    if(dataDir.empty() == true) {
        std::string execName = std::filesystem::path(execPath).filename();
        dataDir = execName + ".data";
    }
    if(managerFlag == true && groupFlag == true) {
        printf("Bad arguments.");
        return ErrCode::InvalidArgument;
    }
    if(groupFlag == false) { // default run as manager
        managerFlag = true;
    }

    return 0;
}

const std::string& OptParser::getExecPath()
{
    return execPath;
}

const std::string& OptParser::getDataDir()
{
    return dataDir;
}

bool OptParser::isManager()
{
    return managerFlag;
}

/* =========================================== */
/* === class protected function implement  === */
/* =========================================== */

/* =========================================== */
/* === class private function implement  ===== */
/* =========================================== */
void OptParser::printUsage()
{
    std::stringstream usage;

    usage << "Carrier Group Daemon." << std::endl;
    usage << std::endl;

    usage << "Usage: " << std::filesystem::path(execPath).filename() << " [OPTION]...";
    usage << std::endl;
    usage << "Run options:" << std::endl;
    for(const auto& it: LongOptions) {
        if(it.opt.name == nullptr) {
            break;
        }
        usage << "  ";
        if(it.opt.val < MaxVisibleOpt) {
            usage << "-" << static_cast<char>(it.opt.val)  << ",";
        } else {
            usage << "   ";
        }

        std::string arg = (it.opt.has_arg ? std::string("=") + it.opt.name : "");
        std::transform(arg.begin(), arg.end(), arg.begin(),
                       [](unsigned char c) { return std::toupper(c); });
        usage << " --"
              << std::left << std::setw(20) << std::setfill(' ')
              << (it.opt.name + arg);

        usage << it.usage << std::endl;
    }

    usage << std::endl;

    printf("%s", usage.str().c_str());
}


} // namespace elastos
