#pragma once

#include "vanguardbot_base.hpp"
#include <functional>


class vanguardbot : public vanguardbot_base
{
public:
	vanguardbot(std::string inHostname, int inPortNumber)
		: vanguardbot_base(inHostname, inPortNumber) {}

	void	log_in(std::string userName, std::string password, std::string channelName);

	void	send_chat_message(std::string msg);
	
	virtual void	handle_command_for_user_with_params_prefix_tags(std::string command, std::string userName, std::vector<std::string> params, std::string prefix, std::string tags);

protected:
	virtual void	process_one_line(std::string currLine);
	virtual void	process_full_lines();

	std::string						mChannelName;
	std::string						mUserName;
};

