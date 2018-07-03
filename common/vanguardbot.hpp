#pragma once

#include "vanguardbot_base.hpp"
#include <functional>
#include <vector>
#include <map>


struct irc_command
{
	std::string					command;
	std::string					userName;
	std::vector<std::string>	params;
	std::string					prefix;
	std::string					tags;
};


typedef std::function<void(const irc_command&)> irc_command_handler;


class vanguardbot : public vanguardbot_base
{
public:
	vanguardbot(std::string inHostname, int inPortNumber, std::function<void()> inReadyToRunHandler);

	void	log_in(std::string userName, std::string password, std::string channelName);

	void	send_chat_message(std::string msg);
	
	// Bot commands are things like "!quote". or "!addquote What are yu doing".
	//	Specify the name without the exclamation mark, specify "*" to be called
	//	for every bot command for which no handler exists.
	void	add_bot_command_handler(std::string name, irc_command_handler handler) { mBotCommandHandlers[name] = handler; };
	
	// Protocol commands are IRC's internal commands, like PRIVMSG or PING.
	void	add_protocol_command_handler(std::string name, irc_command_handler handler) { mProtocolCommandHandlers[name] = handler; };

protected:
	virtual void	process_one_line(std::string currLine);
	virtual void	process_full_lines();

	virtual void	handle_command(const irc_command& inCommand);

	std::string									mChannelName;
	std::string									mUserName;
	std::map<std::string, irc_command_handler>	mProtocolCommandHandlers;
	std::map<std::string, irc_command_handler>	mBotCommandHandlers;
};

