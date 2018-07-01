#include "vanguardbot.hpp"
#include <iostream>
#include <vector>
#include <map>


using namespace std;


void	vanguardbot::send_chat_message(string msg)
{
	string chatCommand("PRIVMSG #");
	chatCommand.append(mChannelName);
	chatCommand.append(" :");
	chatCommand.append(msg);
	send_message(chatCommand);

	string externalChatCommand;
	externalChatCommand.append(":");
	externalChatCommand.append(mUserName);
	externalChatCommand.append("!");
	externalChatCommand.append(mUserName);
	externalChatCommand.append("@");
	externalChatCommand.append(mUserName);
	externalChatCommand.append(".tmi.twitch.tv ");
	externalChatCommand.append(chatCommand);
	process_one_line(externalChatCommand);
}


void	vanguardbot::process_one_line(string currLine)
{
	string currMessage(currLine);
	string userName;
	string prefix;
	string tags;
	vector<string> messageParts;
	
	if (currMessage.find("@") == 0)
	{
		size_t tagsEndOffset = currMessage.find(" ");
		if (tagsEndOffset != string::npos)
		{
			tags = currMessage.substr(1, tagsEndOffset);
			currMessage.erase(0, tagsEndOffset + 1);
		}
	}
	
	size_t firstPartEndOffset = currMessage.find(" ");
	if (firstPartEndOffset != string::npos)
	{
		prefix = currMessage.substr(0, firstPartEndOffset);
		currMessage.erase(0, firstPartEndOffset + 1);
		
		if (prefix.find(":") == 0)
		{
			size_t	userSeparatorOffset = prefix.find("!");
			if(userSeparatorOffset == 0)
			{
				userName = prefix.substr(1, userSeparatorOffset - 1);
				prefix = prefix.substr(userSeparatorOffset + 1, prefix.length() - (userSeparatorOffset + 1));
			}
		}
		else
		{
			messageParts.push_back(prefix);
			prefix.erase();
		}
		
		while (true)
		{
			size_t partEndOffset = currMessage.find(" ");
			if (partEndOffset == string::npos) // No more spaces? Just grab the rest of the string.
			{
				if (currMessage.length() > 0)
				{
					if (currMessage.find(":") == 0) // Remove any "grab rest of string" indicators.
					{
						messageParts.push_back(currMessage.substr(1));
					}
					else
					{
						messageParts.push_back(currMessage);
					}
					break;
				}
			}
			
			string currPart = currMessage.substr(0, partEndOffset);
			if (currMessage.length() > 0 && currPart[0] == ':') // Indicator to grab the rest of the line, spaces and all?
			{
				currPart = currMessage.substr(1, currMessage.length() - 1);
				currMessage.erase();
				messageParts.push_back(currPart);
				break;
			}
			else
			{
				currMessage.erase(0, partEndOffset + 1);
				messageParts.push_back(currPart);
			}
		}
	}
	else
	{
		messageParts.push_back(currMessage);
	}
	
	string commandName((messageParts.size() > 0) ? messageParts.front() : "");
	vector<string> params((messageParts.size() > 1) ? messageParts.begin() + 1 : messageParts.begin(), messageParts.end());
	
	handle_command_for_user_with_params_prefix_tags(commandName, userName, params, prefix, tags);
}


void	vanguardbot::handle_command_for_user_with_params_prefix_tags(std::string command, std::string userName, std::vector<std::string> params, std::string prefix, std::string tags)
{
	cout << "Received: " << userName << ": " << command;
	for (const string& currParam : params)
	{
		cout << " \"" << currParam << "\"";
	}
	cout << "|" << prefix << "|" << tags << endl;
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
	mChannelName = channelName;
	mUserName = userName;
	
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
	
	send_chat_message("Hi.");
}

