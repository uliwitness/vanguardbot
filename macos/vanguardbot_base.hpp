#pragma once

#include <string>
#include <functional>
#include <objc/objc.h>


class vanguardbot_base
{
public:
	vanguardbot_base() {}
	virtual ~vanguardbot_base();
	
	virtual void	connect(const std::string& inHostname, int inPortNumber, const std::string& inFolderPath, std::function<void()> inReadyToRunHandler);

	void	send_message(std::string inMessage);

	void	run() {}

protected:
	virtual void	process_full_lines() = 0;
	virtual void	log_in(std::string userName, std::string password, std::string channelName) = 0;

	std::string		mMessageBuffer;
	id				mIVars = nil;
};

