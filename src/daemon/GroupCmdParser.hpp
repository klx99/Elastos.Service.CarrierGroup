/**
 * @file	CarrierHandler.hpp
 * @brief	CarrierHandler
 * @details	
 *
 * @author	xxx
 * @author	<xxx@xxx.com>
 * @copyright	(c) 2012 xxx All rights reserved.
 **/

#ifndef _ELASTOS_GROUP_CMD_PARSER_HPP_
#define _ELASTOS_GROUP_CMD_PARSER_HPP_

#include <CmdParser.hpp>

namespace elastos {

class GroupCmdParser: public CmdParser {
public:
    /*** type define ***/
    struct Cmd {
        inline static const std::string ListFriend = "/list";
        // inline static const std::string AllowFriend = "/allow";
        inline static const std::string InviteFriend = "/invite";
        inline static const std::string KickFriend = "/kick";
        inline static const std::string ForwardMessage = "/forward";
    };

    /*** static function and variable ***/

    /*** class function and variable ***/

protected:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/
    explicit GroupCmdParser() = default;
    virtual ~GroupCmdParser() = default;

    virtual std::vector<CommandInfo> getCmdInfoList() override;

private:
    /*** type define ***/

    /*** static function and variable ***/
    static const std::string PromptKicked;

    /*** class function and variable ***/

    int onListFriend(const std::weak_ptr<Carrier>& carrier,
                     const std::vector<std::string>& args,
                     const std::string& controller, int64_t timestamp);
    int onInviteFriend(const std::weak_ptr<Carrier>& carrier,
                       const std::vector<std::string>& args,
                       const std::string& controller, int64_t timestamp);
    int onKickFriend(const std::weak_ptr<Carrier>& carrier,
                     const std::vector<std::string>& args,
                     const std::string& controller, int64_t timestamp);
    int onForwardMessage(const std::weak_ptr<Carrier>& carrier,
                         const std::vector<std::string>& args,
                         const std::string& controller, int64_t timestamp);
    
    int forwardMsgToAllFriends(const std::weak_ptr<Carrier>& carrier);
    int forwardMsgToFriend(const std::weak_ptr<Carrier>& carrier, const std::string& friendId);
}; // class GroupCmdParser

} // namespace elastos

#endif /* _ELASTOS_GROUP_CMD_PARSER_HPP_ */