#include "vanguardbot.hpp"
#include "ini_file.hpp"
#include <iostream>
#include <time.h>
#ifdef _WIN32
#include <filesystem>
#else
#include "fake_filesystem.hpp"
#endif
#include <fstream>


using namespace std;

#ifdef _WIN32
using namespace std::experimental::filesystem::v1;
#else
using namespace fake::filesystem;
#endif


void	vanguardbot::connect(const std::string& inHostname, int inPortNumber, const std::string& inFolderPath, std::function<void()> inReadyToRunHandler)
{
	vanguardbot_base::connect(inHostname, inPortNumber, inFolderPath, [inReadyToRunHandler, this, inFolderPath]()
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
								  
								  add_bot_command_handler("*", [this](irc_command inCommand)
														  {
															  send_chat_message("This looks like nothing to me.");
														  });
								  
								  path	commandsFolderPath(inFolderPath);
								  if (exists(commandsFolderPath))
								  {
									  directory_iterator	directoryIterator(commandsFolderPath);
									  for ( ; directoryIterator != directory_iterator(); ++directoryIterator )
									  {
										  const directory_entry& currFile = *directoryIterator;
										  if (currFile.path().filename().string().compare("data") == 0 || currFile.path().filename().string().find(".") == 0)
										  {
											  continue;
										  }
										  load_one_command_folder(currFile.path().string());
									  }
								  }
								  else
								  {
									  cout << "No directory " << commandsFolderPath.string() << endl;
								  }

								  
								  inReadyToRunHandler();
							  });
}


void	vanguardbot::load_one_command_folder(const string &inCommandFolder)
{
	path		commandFolder(inCommandFolder);
	string		commandName(commandFolder.filename().string());
	path		iniFilePath(commandFolder / "info.ini");
	ini_file	commandInfo(iniFilePath.string());

	string commandType = commandInfo.value_for_key("type");

	if (commandType.compare("quote") == 0)
	{
		cout << "Adding command: " << commandName << " (" << commandType << ")" << endl;

		string	addCommandName = commandInfo.value_for_key("addcommand");
		string	addCommandPattern = commandInfo.value_for_key("addcommandpattern");
		string	pattern = commandInfo.value_for_key("pattern");

		string	quotesFileName = commandInfo.value_for_key("filename");
		if (quotesFileName.empty())
		{
			quotesFileName = "quotes.txt";
		}
		path	quotesFilePath(commandFolder.parent_path() / "data" / quotesFileName);

		add_bot_command_handler(commandName, [this, quotesFilePath, pattern](irc_command inCommand)
		{
			irc_command cmd = apply_pattern_to_command(pattern, inCommand);
			vector<string> lines;
			ifstream quotesFile(quotesFilePath.string());
			while (quotesFile.good())
			{
				char currLine[1024] = {};
				quotesFile.getline(currLine, sizeof(currLine) - 1);
				lines.push_back(currLine);
			}

			if (!lines.empty())
			{
				size_t idx = rand() % lines.size();
				send_chat_message(lines[idx]);
			}
		});
		
		if (addCommandName.length() > 0)
		{
			add_bot_command_handler(addCommandName, [this, quotesFilePath, addCommandPattern](irc_command inCommand)
									{
										irc_command cmd = apply_pattern_to_command(addCommandPattern, inCommand);
										if (cmd.params.size() > 0 && cmd.params[0].length() > 0)
										{
											ofstream quotesFile(quotesFilePath.string(), ios::app);
											quotesFile << cmd.params[0] << endl;
										}
									});
		}
	}
}


void replace_with_in(string pattern, string replacement, string &target)
{
	size_t currPos = 0;
	while(currPos < target.length())
	{
		size_t foundPos = target.find(pattern, currPos);
		if (foundPos == string::npos)
		{
			return;
		}
		
		target.replace(foundPos, pattern.length(), replacement);
		currPos = foundPos + replacement.length();
	}
}


vector<string>	split_string_at(string inTarget, string splitter)
{
	vector<string> result;
	
	size_t currPos = 0;
	while (currPos < inTarget.length())
	{
		size_t separatorPos = inTarget.find(splitter, currPos);
		if (separatorPos == string::npos)
		{
			separatorPos = inTarget.length();
		}
		result.push_back(inTarget.substr(currPos, separatorPos - currPos));
		
		currPos = separatorPos + splitter.length();
	}
	
	return result;
}


irc_command	vanguardbot::apply_pattern_to_command(const string& pattern, const irc_command &inCommand)
{
	irc_command		tempCommand = inCommand;
	
	if (pattern.length() > 0)
	{
		tempCommand.params.clear();

		size_t currPos = 0;
		while (currPos < pattern.length())
		{
			size_t destNameEndOffs = pattern.find("=", currPos);
			if (destNameEndOffs != string::npos)
			{
				string destName = pattern.substr(currPos + 1, destNameEndOffs - (currPos + 1));	// +1 to remove "$" sign.
				
				currPos = destNameEndOffs + 1;
				
				size_t patternToMatchEndOffs = pattern.find(",", currPos);
				if (patternToMatchEndOffs == string::npos)
				{
					patternToMatchEndOffs = pattern.length();
				}
				
				string patternToMatch = pattern.substr(currPos, patternToMatchEndOffs - currPos);
				
				currPos = patternToMatchEndOffs + 1;
				string paramsStr = (inCommand.params.size() > 0) ? inCommand.params[0] : "";
				
				vector<string> botParams = split_string_at(paramsStr, " ");
				
				replace_with_in("$USERNAME", inCommand.userName, patternToMatch);
				replace_with_in("$_", paramsStr, patternToMatch);
				for (size_t x = 0; x < botParams.size(); ++x)
				{
					string currIndexStr("$");
					currIndexStr.append(to_string(x + 1));
					replace_with_in(currIndexStr, botParams[x], patternToMatch);
				}

				char* endPtr = NULL;
				long paramNo = strtol(destName.c_str(), &endPtr, 10);
				while(tempCommand.params.size() < paramNo)
				{
					tempCommand.params.push_back("");
				}
				tempCommand.params[paramNo - 1] = patternToMatch;

				cout << "pattern: " << patternToMatch << "-->" << paramNo << endl;
			}
			else
				break;
		}
	}
	
	return tempCommand;
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
			if(userSeparatorOffset != string::npos)
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
		irc_command botCommand = inCommand;
		botCommand.params.clear();
		string paramsStr(inCommand.params[1]);	// 0 is channel name.
		if (paramsStr.length() > 1 && paramsStr[0] == '!')
		{
			size_t partSeparatorOffset = paramsStr.find(" ");
			if (partSeparatorOffset == string::npos)
				partSeparatorOffset = paramsStr.length();
			
			botCommand.command = paramsStr.substr(1, partSeparatorOffset - 1);
			if ((partSeparatorOffset + 1) < paramsStr.length())
			{
				botCommand.params.push_back(paramsStr.substr(partSeparatorOffset + 1, paramsStr.length() - (partSeparatorOffset - 1)));
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


void	vanguardbot::log_out()
{
	string partMsg("PART #");
	partMsg.append(mChannelName);
	send_message(partMsg);
	
	send_message("QUIT :bot is shutting down.");
}
