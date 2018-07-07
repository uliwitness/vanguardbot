#pragma once

#include <string>
#include <functional>


class vanguardbot_base
{
public:
	vanguardbot_base() {}
	virtual ~vanguardbot_base();
	
	virtual void	connect(const std::string &inHostname, int inPortNumber, std::function<void()> inReadyToRunHandler);

	void	send_message(std::string inMessage);

	void	run();

protected:
	void			read_once();
	virtual void	process_full_lines() = 0;
	virtual void	log_in(std::string userName, std::string password, std::string channelName) = 0;

	bool										mKeepRunning = true;
	std::string									mMessageBuffer;
	Windows::Networking::Sockets::StreamSocket	^mSocket;
	Windows::Storage::Streams::DataWriter		^mDataWriter;
	Windows::Storage::Streams::DataReader		^mDataReader;
};

