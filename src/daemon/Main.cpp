#include <csignal>
#include <thread>

#include <Daemon.hpp>
#include <ErrCode.hpp>
#include <Log.hpp>
#include <OptParser.hpp>

static void SignalHandler(int signal);
static bool gExitFlag = false;

int main(int argc, char **argv)
{
    using namespace elastos;

    std::signal(SIGINT, SignalHandler);
    std::signal(SIGSEGV, SignalHandler);
    std::signal(SIGTERM, SignalHandler);
    std::signal(SIGKILL, SignalHandler);

    int rc = OptParser::GetInstance()->parse(argc, argv);
    CHECK_ERROR(rc);

    rc = Daemon::GetInstance()->run();
    CHECK_ERROR(rc);

    while(gExitFlag == false) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}

static void SignalHandler(int signal)
{
    gExitFlag = true;
}
