#pragma once

#include <string>
#include <functional>
#include <objc/objc.h>
#include <chrono>


namespace vanguard {

using namespace std;
	
class vanguardbot_base
{
public:
	vanguardbot_base() {}
	virtual ~vanguardbot_base();
	
	virtual void	connect(const string& inHostname, int inPortNumber, const string& inFolderPath, function<void()> inReadyToRunHandler);

	void	send_message(string inMessage);

	void	run() {}
	
	//! Perform the given block after the given delay.
	void	perform_after(chrono::minutes delay, bool repeat, function<void()> handler);

protected:
	virtual void	process_full_lines() = 0;
	virtual void	log_in(string userName, string password, string channelName) = 0;

	string		mMessageBuffer;
	id			mIVars = nil;
};

} /* namespace vanguard */
