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
	vanguardbot(std::string inHostname, int inPortNumber);

	void	log_in(std::string userName, std::string password, std::string channelName);

	void	send_chat_message(std::string msg);
	
	void	add_protocol_command_handler(std::string name, irc_command_handler handler) { mProtocolCommandHandlers[name] = handler; };
	
	virtual void	handle_command(const irc_command& inCommand);

protected:
	virtual void	process_one_line(std::string currLine);
	virtual void	process_full_lines();

	std::string									mChannelName;
	std::string									mUserName;
	std::map<std::string, irc_command_handler>	mProtocolCommandHandlers;
};

