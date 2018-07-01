#pragma once

#include "vanguardbot_base.hpp"
#include <functional>

class vanguardbot : public vanguardbot_base
{
public:
	vanguardbot(std::string inHostname, int inPortNumber, std::string userName, std::string password, std::string channelName)
		: vanguardbot_base(inHostname, inPortNumber, userName, password, channelName) {}


	void	set_line_handler(std::function<void(std::string)> inHandler) { mLineHandler = inHandler; }

protected:
	virtual void	process_one_line(std::string currLine);
	virtual void	process_full_lines();

	std::function<void(std::string)>	mLineHandler;
};

