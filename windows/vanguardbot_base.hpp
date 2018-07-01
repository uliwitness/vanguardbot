#pragma once

#include <winsock.h>
#include <string>
#include <functional>


class vanguardbot_base
{
public:
	vanguardbot_base(std::string inHostname, int inPortNumber, std::string userName, std::string password, std::string channelName);
	virtual ~vanguardbot_base();

	void	send_message(std::string inMessage);

	void	run();

protected:
	virtual void	process_full_lines() = 0;

	bool								mKeepRunning = true;
	std::string							mMessageBuffer;
	SOCKET								mSocket = INVALID_SOCKET;
};

