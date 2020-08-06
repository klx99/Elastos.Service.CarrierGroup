/**
 * @file	CarrierHandler.hpp
 * @brief	CarrierHandler
 * @details	
 *
 * @author	xxx
 * @author	<xxx@xxx.com>
 * @copyright	(c) 2012 xxx All rights reserved.
 **/

#ifndef _ELASTOS_OPT_PARSER_HPP_
#define _ELASTOS_OPT_PARSER_HPP_

#include <getopt.h>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace elastos {

class Carrier;

class OptParser {
public:
    /*** type define ***/
    struct OptName {
        inline static const char* Help = "help";
        inline static const char* DataDir = "datadir";
        inline static const char* Manager = "manager";
        inline static const char* Group = "group";
    };

    /*** static function and variable ***/
    static std::shared_ptr<OptParser> GetInstance();

    /*** class function and variable ***/
    int parse(int argc, char **argv);

    const std::string& getExecPath();
    const std::string& getDataDir();
    bool isManager();

protected:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/

private:
    /*** type define ***/

    /*** static function and variable ***/
    static std::recursive_mutex OptMutex;
    static std::shared_ptr<OptParser> OptParserInstance;

    static const int MaxVisibleOpt;
    static const char* ShortOptions;
    static struct Option {
        struct option opt;
        const char* usage;
    } LongOptions[];

    /*** class function and variable ***/
    explicit OptParser() = default;
    virtual ~OptParser() = default;

    void printUsage();

    std::string execPath;
    std::vector<const char*> execArgs;

    bool managerFlag;
    std::string dataDir;
}; // class OptParser

} // namespace elastos

#endif /* _ELASTOS_OPT_PARSER_HPP_ */