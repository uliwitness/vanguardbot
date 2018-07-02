#include "vanguardbot.hpp"
#include <iostream>


using namespace std;


vanguardbot::vanguardbot(std::string inHostname, int inPortNumber)
	: vanguardbot_base(inHostname, inPortNumber)
{
	srand((unsigned)time(NULL));
	
	add_protocol_command_handler("PING", [this](irc_command inCommand)
	{
		string pongMessage("PONG ");
		if (inCommand.params.size() > 0)
		{
			pongMessage.append(inCommand.params.front());
		}
		send_message(pongMessage);
	});
	
	add_protocol_command_handler("*", [this](irc_command inCommand)
	{
		cout << "Received: " << inCommand.userName << ": " << inCommand.command;
		for (const string& currParam : inCommand.params)
		{
			cout << " \"" << currParam << "\"";
		}
		cout << "|" << inCommand.prefix << "|" << inCommand.tags << endl;
	});

	add_bot_command_handler("fine", [this](irc_command inCommand)
	{
		if (rand() % 2)
		{
			send_chat_message("Everything is fine. Nice blinking lights.");
		}
		else
		{
			send_chat_message("Everything is fi-- fire! It is everywhere! It Burns!");
		}
	});

	add_bot_command_handler("*", [this](irc_command inCommand)
	{
		send_chat_message("This looks like nothing to me.");
	});
}


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
	irc_command command;
	vector<string> messageParts;
	
	if (currMessage.find("@") == 0)
	{
		size_t tagsEndOffset = currMessage.find(" ");
		if (tagsEndOffset != string::npos)
		{
			command.tags = currMessage.substr(1, tagsEndOffset);
			currMessage.erase(0, tagsEndOffset + 1);
		}
	}
	
	size_t firstPartEndOffset = currMessage.find(" ");
	if (firstPartEndOffset != string::npos)
	{
		command.prefix = currMessage.substr(0, firstPartEndOffset);
		currMessage.erase(0, firstPartEndOffset + 1);
		
		if (command.prefix.find(":") == 0)
		{
			size_t	userSeparatorOffset = command.prefix.find("!");
			if(userSeparatorOffset == 0)
			{
				command.userName = command.prefix.substr(1, userSeparatorOffset - 1);
				command.prefix = command.prefix.substr(userSeparatorOffset + 1, command.prefix.length() - (userSeparatorOffset + 1));
			}
		}
		else
		{
			messageParts.push_back(command.prefix);
			command.prefix.erase();
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
	
	command.command = string((messageParts.size() > 0) ? messageParts.front() : "");
	command.params = vector<string>((messageParts.size() > 1) ? messageParts.begin() + 1 : messageParts.begin(), messageParts.end());
	
	handle_command(command);
}


void	vanguardbot::handle_command(const irc_command& inCommand)
{
	map<string, irc_command_handler>::iterator foundHandler = mProtocolCommandHandlers.find(inCommand.command);
	if (foundHandler != mProtocolCommandHandlers.end())
	{
		foundHandler->second(inCommand);
	}
	else if (inCommand.command.compare("PRIVMSG") == 0 && inCommand.params.size() > 1)
	{
		irc_command botCommand;
		string paramsStr(inCommand.params[1]);	// 0 is channel name.
		if (paramsStr.length() > 1 && paramsStr[0] == '!')
		{
			size_t partSeparatorOffset = paramsStr.find(" ");
			if (partSeparatorOffset == string::npos)
				partSeparatorOffset = paramsStr.length();
			
			botCommand.command = paramsStr.substr(1, partSeparatorOffset);
			if ((partSeparatorOffset + 1) < paramsStr.length())
			{
				botCommand.params.push_back(paramsStr.substr(partSeparatorOffset, paramsStr.length() - partSeparatorOffset));
			}
			
			map<string, irc_command_handler>::iterator foundHandler = mBotCommandHandlers.find(botCommand.command);
			if (foundHandler != mBotCommandHandlers.end())
			{
				foundHandler->second(botCommand);
			}
			else
			{
				foundHandler = mBotCommandHandlers.find("*");
				if (foundHandler != mBotCommandHandlers.end())
				{
					foundHandler->second(botCommand);
				}
				else
				{
					foundHandler = mProtocolCommandHandlers.find("*");
					if (foundHandler != mProtocolCommandHandlers.end())
					{
						foundHandler->second(inCommand);
					}
				}
			}
		}
		else
		{
			foundHandler = mProtocolCommandHandlers.find("*");
			if (foundHandler != mProtocolCommandHandlers.end())
			{
				foundHandler->second(inCommand);
			}
		}
	}
	else
	{
		foundHandler = mProtocolCommandHandlers.find("*");
		if (foundHandler != mProtocolCommandHandlers.end())
		{
			foundHandler->second(inCommand);
		}
	}
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

