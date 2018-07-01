#include "vanguardbot.hpp"
#include <iostream>


using namespace std;


void	vanguardbot::process_one_line(string currLine)
{
	mLineHandler(currLine);
}


void	vanguardbot::process_full_lines()
{
	while (true)
	{
		size_t lineBreakPos = mMessageBuffer.find("\r\n");
		if (lineBreakPos != string::npos)
		{
			string currLine(mMessageBuffer.substr(0, lineBreakPos));
			mMessageBuffer.erase(0, lineBreakPos + 2);

			process_one_line(currLine);
		}
		else
		{
			break;
		}
	}
}


void	vanguardbot::log_in(std::string userName, std::string password, std::string channelName)
{
	string passMsg("PASS ");
	passMsg.append(password);
	send_message(passMsg);
	string nickMsg("NICK ");
	nickMsg.append(userName);
	send_message(nickMsg);
	send_message("CAP REQ : twitch.tv / membership");
	send_message("CAP REQ : twitch.tv / tags");
	send_message("CAP REQ : twitch.tv / commands");
	string joinMsg("JOIN #");
	joinMsg.append(channelName);
	send_message(joinMsg);
}

