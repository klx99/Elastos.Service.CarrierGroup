/**
 * @file	CarrierHandler.hpp
 * @brief	CarrierHandler
 * @details	
 *
 * @author	xxx
 * @author	<xxx@xxx.com>
 * @copyright	(c) 2012 xxx All rights reserved.
 **/

#ifndef _ELASTOS_MANAGER_CMD_PARSER_HPP_
#define _ELASTOS_MANAGER_CMD_PARSER_HPP_

#include <CmdParser.hpp>

namespace elastos {

class ManagerCmdParser: public CmdParser {
public:
    /*** type define ***/
    struct Cmd {
        inline static const std::string NewGroup = "/new";
        inline static const std::string StartGroup = "/start";
        inline static const std::string ListGroup = "/list";
        inline static const std::string Forward = "/forward";
    };

    /*** static function and variable ***/

    /*** class function and variable ***/

protected:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/
    explicit ManagerCmdParser() = default;
    virtual ~ManagerCmdParser() = default;

    virtual std::vector<CommandInfo> getCmdInfoList() override;

private:
    /*** type define ***/

    /*** static function and variable ***/
    static const int MaxWaitNewGroupTime;

    /*** class function and variable ***/
    int onNewGroup(const std::weak_ptr<Carrier>& carrier,
                   const std::vector<std::string>& args,
                   const std::string& controller, int64_t timestamp);
    int onStartGroup(const std::weak_ptr<Carrier>& carrier,
                     const std::vector<std::string>& args,
                     const std::string& controller, int64_t timestamp);
    int onListGroup(const std::weak_ptr<Carrier>& carrier,
                    const std::vector<std::string>& args,
                    const std::string& controller, int64_t timestamp);

}; // class ManagerCmdParser

} // namespace elastos

#endif /* _ELASTOS_MANAGER_CMD_PARSER_HPP_ */