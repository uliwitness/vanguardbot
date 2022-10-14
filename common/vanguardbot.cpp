#include "vanguardbot.hpp"
#include "ini_file.hpp"
#include <iostream>
#include <time.h>
#ifndef _WIN32
#include <unistd.h>
#include "twitch.hpp"
#endif
#include <filesystem>
#include <fstream>
#include "string_utils.hpp"

namespace vanguard {

    using namespace std;
    using namespace vanguard;

#ifdef _WIN32
	using namespace std::experimental::filesystem::v1;
#else
	using namespace std::filesystem;
#endif
	
	
	static string field_from_map(const string& key, const map<string, string>& tags, const string& fallback = string())
	{
		auto foundBadge = tags.find(key);
		return (foundBadge == tags.end() ? fallback : foundBadge->second);
	}

	void	vanguardbot::connect(const std::string& inHostname, int inPortNumber, const std::string& inFolderPath, std::function<void()> inReadyToRunHandler)
	{
		mCommandsFolderPath = inFolderPath;
		ofstream todaySeenUsers(mCommandsFolderPath + "/data/todayseenusers.txt", ios_base::trunc);

		ifstream everSeenUsers(mCommandsFolderPath + "/data/seenusers.txt", ios_base::in);
		mEverSeenUsers.erase(mEverSeenUsers.begin(), mEverSeenUsers.end());
		char buffer[1024] = {};
		if( everSeenUsers.is_open() )
		{
			while( true )
			{
				everSeenUsers.getline(buffer, sizeof(buffer));
				if( everSeenUsers.eof() || everSeenUsers.bad() )
				{
					break;
				}
				mEverSeenUsers.insert(string(buffer));
			}
		}

		vanguardbot_base::connect(inHostname, inPortNumber, [inReadyToRunHandler, this, inFolderPath]()
								  {
			srand((unsigned)time(NULL));
			
			add_protocol_command_handler("PING", [this](irc_command inCommand)
										 {
				string pongMessage("PONG ");
				if( inCommand.params.size() > 0 )
				{
					pongMessage.append(inCommand.params.front());
				}
				send_message(pongMessage);
			});
			
			add_protocol_command_handler("NOTICE", [this](irc_command inCommand)
										 {
				if( inCommand.params.size() > 1 && mNoticeHandler )
				{
					mNoticeHandler(inCommand.params[1]);
				}
			});

            set_privmsg_handler([this](irc_command inCommand)
            {
                cout << inCommand.userName << ": " << (inCommand.params.size() > 1 ? inCommand.params[1].c_str() : "") << endl;
            });

			add_protocol_command_handler("*", [this](irc_command inCommand)
			{
#if VERBOSE_LOGGING
				cout << "Received: " << inCommand.userName << ": " << inCommand.command;
				for( const string& currParam : inCommand.params )
				{
					cout << " \"" << currParam << "\"";
				}

				string tagsString("\n");
				for( auto tag : inCommand.tags )
				{
					tagsString += string("\t") + tag.first + ": " + tag.second + "\n";
				}
				if( tagsString.length() == 1 )
				{
					tagsString.erase();
				}
				cout << "|" << inCommand.prefix << "\n[" << tagsString << "]" << endl;
#endif
			});

			add_protocol_command_handler("USERSTATE", [this](irc_command inCommand)
										 {
				string userName(tolower(inCommand.tags["display-name"]));
                map<string,string>& userTags = mUserTags[userName];
                userTags = inCommand.tags;

                userTags[";management"] = (field_from_map("mod", inCommand.tags) == "1" || userName == tolower(mChannelName)) ? "1" : "0";

#if VERBOSE_LOGGING
				cout << "User State: " << userName << ":\n";
				for( auto tagPair : userTags )
				{
					cout << "\t" << tagPair.first << ": " << tagPair.second << "\n";
				}
#endif
			});

			add_protocol_command_handler("ROOMSTATE", [this](irc_command inCommand)
										 {
				cout << "Room State:\n";
				for( auto tagPair : inCommand.tags )
				{
					cout << "\t" << tagPair.first << ": " << tagPair.second << "\n";
				}
				
				mRoomTags = inCommand.tags;
			});

			add_bot_command_handler("*", [this](irc_command inCommand)
			{
				send_chat_message("This looks like nothing to me.");
			}, false, false);

            add_bot_command_handler("quit", [this](irc_command inCommand)
            {
                send_chat_message("OK, I'm leaving. Have a great day.");
                set_keep_running(false);
            }, true, false);

            path	commandsFolderPath(inFolderPath);
			if (exists(commandsFolderPath))
			{
				directory_iterator	directoryIterator(commandsFolderPath);
				for ( ; directoryIterator != directory_iterator(); ++directoryIterator )
				{
					const directory_entry& currFile = *directoryIterator;
					if (currFile.path().filename().string() == "data" || currFile.path().filename().string().find('.') == 0)
					{
						continue;
					}
					load_one_command_folder(currFile.path().string());
				}
			}
			else
			{
				cerr << "No directory " << commandsFolderPath.string() << endl;
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
		bool mustBeManagement = tolower(commandInfo.value_for_key("mustBeManagement")) == "true";
		bool mustBeSubscriber = tolower(commandInfo.value_for_key("mustBeSubscriber")) == "true";

		if (commandType.compare("quote") == 0) {
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
				
				if (!lines.empty() )
				{
					size_t idx = 0;
					if(inCommand.params.size() == 1)
					{
						char* endPtr = NULL;
						idx = strtoul(inCommand.params[0].c_str(), &endPtr, 10);
						if (idx < lines.size())
						{
							send_chat_message(lines[idx]);
						}
					}
					else
					{
						idx = rand() % lines.size();
						send_chat_message(lines[idx]);
					}
				}
			}, mustBeManagement, mustBeSubscriber);
			
			if (addCommandName.length() > 0)
			{
				bool addMustBeManagement = tolower(commandInfo.value_for_key("addCommandMustBeManagement")) == "true";
				bool addMustBeSubscriber = tolower(commandInfo.value_for_key("addCommandMustBeSubscriber")) == "true";

				add_bot_command_handler(addCommandName, [this, quotesFilePath, addCommandPattern](irc_command inCommand)
										{
					irc_command cmd = apply_pattern_to_command(addCommandPattern, inCommand);
					if (cmd.params.size() > 0 && cmd.params[0].length() > 0)
					{
						ofstream quotesFile(quotesFilePath.string(), ios::app);
						quotesFile << join_strings_with(cmd.params, " ") << endl;
					}
				}, addMustBeManagement, addMustBeSubscriber);
			}
		} else if (commandType.compare("counter") == 0) {
			cout << "Adding command: " << commandName << " (" << commandType << ")" << endl;
			
			string	addCommandName = commandInfo.value_for_key("incrementcommand");
			
			string	quotesFileName = commandInfo.value_for_key("filename");
			if (quotesFileName.empty())
			{
				quotesFileName = "counter.txt";
			}
			path	quotesFilePath(commandFolder.parent_path() / "data" / quotesFileName);
			string	response = commandInfo.value_for_key("response");
			string	incrementResponse = commandInfo.value_for_key("incrementresponse");

			add_bot_command_handler(commandName, [this, quotesFilePath, response](irc_command inCommand)
									{
				size_t currentCount = 0;
				ifstream quotesFile(quotesFilePath.string());
				if (quotesFile.good())
				{
					char currLine[1024] = {};
					quotesFile.getline(currLine, sizeof(currLine) - 1);
					currentCount = atoll(currLine);
				}
				
				string msg(response.empty() ? "The counter is at $COUNT." : response);
				replace_with_in("$COUNT", to_string(currentCount), msg);
				replace_with_in("$CHANNELNAME", mChannelName, msg);
				replace_with_in("$USERNAME", inCommand.userName, msg);
				send_chat_message(msg);
			}, mustBeManagement, mustBeSubscriber);
			
			if( addCommandName.length() > 0 )
			{
				bool addMustBeManagement = tolower(commandInfo.value_for_key("incrementCommandMustBeManagement")) == "true";
				bool addMustBeSubscriber = tolower(commandInfo.value_for_key("incrementCommandMustBeSubscriber")) == "true";

				add_bot_command_handler(addCommandName, [this, quotesFilePath, incrementResponse](irc_command inCommand)
										{
					size_t currentCount = 0;
					ifstream quotesFile(quotesFilePath.string());
					if (quotesFile.good())
					{
						char currLine[1024] = {};
						quotesFile.getline(currLine, sizeof(currLine) - 1);
						currentCount = atoll(currLine);
					}

					++currentCount;
					
					ofstream counterFile(quotesFilePath.string(), ios::trunc | ios::out);
					counterFile << to_string(currentCount) << endl;
					
					string msg(incrementResponse.empty() ? "The counter has increased to $COUNT." : incrementResponse);
					replace_with_in("$COUNT", to_string(currentCount), msg);
					replace_with_in("$CHANNELNAME", mChannelName, msg);
					replace_with_in("$USERNAME", inCommand.userName, msg);
					send_chat_message(msg);
				}, addMustBeManagement, addMustBeSubscriber);
			}
		} else if (commandType.compare("timer") == 0) {
            cout << "Adding command: " << commandName << " (" << commandType << ")" << endl;

            string message = commandInfo.value_for_key("message");
            chrono::minutes delay = chrono::minutes(atoll(commandInfo.value_for_key("intervalminutes").c_str()));

            if (message.find("!") == 0) {
                perform_after(delay, true, [this, message]() {
                    send_chat_message(message, true); // Only shows result of the command.
                });
            } else {
                perform_after(delay, true, [this, message]() {
                    send_chat_message(message);
                });
            }
        }
#ifndef _WIN32
        else if (commandType.compare("shell") == 0) {
            cout << "Adding command: " << commandName << " (" << commandType << ")" << endl;

            string			shellCmd = commandInfo.value_for_key("command");
            string			message = commandInfo.value_for_key("message");

            add_bot_command_handler(commandName, [this, shellCmd, message, inCommandFolder](irc_command inCommand)
            {
                if (!message.empty()) {
                    string msg(message);
                    replace_with_in("$CHANNELNAME", mChannelName, msg);
                    replace_with_in("$USERNAME", inCommand.userName, msg);
                    send_chat_message(msg);
                }
                chdir(inCommandFolder.c_str());
                int resultStatus = system(shellCmd.c_str());
                cout << "Executed '" << shellCmd << "' -> " << resultStatus << endl;
            }, mustBeManagement, mustBeSubscriber);
        }
#endif
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
					string paramsStr = join_strings_with(inCommand.params, " ");
					
					time_t rawtime;
					time( &rawtime );
					struct tm *info = localtime( &rawtime );
					char dateStr[512] = {};
					strftime(dateStr, sizeof(dateStr) - 1, "%x", info);
					replace_with_in("$DATE", dateStr, patternToMatch);
					strftime(dateStr, sizeof(dateStr) - 1, "%R", info);
					replace_with_in("$TIME", dateStr, patternToMatch);
					replace_with_in("$USERNAME", inCommand.userName, patternToMatch);
					replace_with_in("$_", paramsStr, patternToMatch);
					
					vector<string> botParams = split_string_at(paramsStr, " ");
					
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
				}
				else
					break;
			}
		}
		
		return tempCommand;
	}
	
	
	void	vanguardbot::send_chat_message(const string& msg, bool invisible)
	{
		string chatCommand("PRIVMSG #");
		chatCommand.append(mChannelName);
		chatCommand.append(" :");
		chatCommand.append(msg);
		if( !invisible )
		{
			send_message(chatCommand);
		}
		
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
	
	void	vanguardbot::process_one_line(const string& currLine)
	{
		string currMessage(currLine);
		irc_command command;
		vector<string> messageParts;
		
		if (currMessage.find("@") == 0)
		{
			size_t tagsEndOffset = currMessage.find(" ");
			if( tagsEndOffset != string::npos )
			{
				command.tagsString = currMessage.substr(1, tagsEndOffset);
				currMessage.erase(0, tagsEndOffset + 1);
				
				auto tagPairs = split_string_at(command.tagsString, ";");
				for( string tagValue : tagPairs )
				{
					string tagName = chomp_until_separator_from_string("=", tagValue);
					command.tags[tagName] = tagValue;
				}
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
		map<string, irc_command_handler>::iterator foundProtocolHandler = mProtocolCommandHandlers.find(inCommand.command);
		if (foundProtocolHandler != mProtocolCommandHandlers.end())
		{
			foundProtocolHandler->second(inCommand);
		}
		else if (inCommand.command.compare("PRIVMSG") == 0 && inCommand.params.size() > 1)
		{
			string userName(tolower(inCommand.userName));
#if VERBOSE_LOGGING
			cout << "User State: " << userName << ":\n";
#endif
			map<string,string>& userTags = mUserTags[userName];

			userTags["badges"] = field_from_map("badges", inCommand.tags);
			userTags["color"] = field_from_map("color", inCommand.tags);

            userTags[";management"] = (field_from_map("mod", inCommand.tags) == "1" || userName == tolower(mChannelName)) ? "1" : "0";

#if VERBOSE_LOGGING
			cout << "User State: " << userName << ":\n";
			for( auto tagPair : userTags )
			{
				cout << "\t" << tagPair.first << ": " << tagPair.second << "\n";
			}
#endif

			string currUser(tolower(inCommand.userName));
			if( mEverSeenUsers.find(currUser) == mEverSeenUsers.end() )
			{
				mTodaySeenUsers.insert(inCommand.userName);
				mEverSeenUsers.insert(inCommand.userName);
				ofstream everSeenUsers(mCommandsFolderPath + "/data/seenusers.txt", ios_base::app);
				everSeenUsers << inCommand.userName << endl;
				if(currUser != tolower(mUserName) && currUser != tolower(mChannelName))
				{
					ofstream todaySeenUsers(mCommandsFolderPath + "/data/todayseenusers.txt", ios_base::app);
					todaySeenUsers << inCommand.userName << endl;
					mEverSeenUserHandler(inCommand.userName);
				}
			}
			else if( mTodaySeenUsers.find(tolower(inCommand.userName)) == mTodaySeenUsers.end() )
			{
				mTodaySeenUsers.insert(inCommand.userName);
				if(currUser != tolower(mUserName) && currUser != tolower(mChannelName))
				{
					ofstream todaySeenUsers(mCommandsFolderPath + "/data/todayseenusers.txt", ios_base::app);
					todaySeenUsers << inCommand.userName << endl;
					mTodaySeenUserHandler(inCommand.userName);
				}
			}
			
			irc_command botCommand = inCommand;
			botCommand.params.clear();
			string paramsStr(inCommand.params[1]);	// 0 is channel name.
			if( paramsStr.length() > 1 && paramsStr[0] == '!' )
			{
				size_t partSeparatorOffset = paramsStr.find(' ');
				if (partSeparatorOffset == string::npos)
					partSeparatorOffset = paramsStr.length();
				
				botCommand.command = tolower(paramsStr.substr(1, partSeparatorOffset - 1));
				if ((partSeparatorOffset + 1) < paramsStr.length())
				{
					botCommand.params.push_back(paramsStr.substr(partSeparatorOffset + 1, paramsStr.length() - (partSeparatorOffset - 1)));
				}
				
				auto foundHandler = mBotCommandHandlers.find(botCommand.command);
				if (foundHandler != mBotCommandHandlers.end()
					&& (!foundHandler->second.mustBeSubscriber || userTags["subscriber"] == "1")
					&& (!foundHandler->second.mustBeManagement || userTags[";management"] == "1"))
				{
					foundHandler->second.handler(botCommand);
				}
				else
				{
					foundHandler = mBotCommandHandlers.find("*");
					if (foundHandler != mBotCommandHandlers.end()
						&& (!foundHandler->second.mustBeSubscriber || userTags["subscriber"] == "1")
						&& (!foundHandler->second.mustBeManagement || userTags[";management"] == "1"))
					{
						foundHandler->second.handler(botCommand);
					}
					else
					{
						if( mPrivMsgHandler )
						{
							mPrivMsgHandler(inCommand);
						}
						else
						{
							foundProtocolHandler = mProtocolCommandHandlers.find("*");
							if (foundProtocolHandler != mProtocolCommandHandlers.end())
							{
								foundProtocolHandler->second(inCommand);
							}
						}
					}
				}
			}
			else
			{
				if( mPrivMsgHandler )
				{
					mPrivMsgHandler(inCommand);
				}
				else
				{
					foundProtocolHandler = mProtocolCommandHandlers.find("*");
					if (foundProtocolHandler != mProtocolCommandHandlers.end())
					{
						foundProtocolHandler->second(inCommand);
					}
				}
			}
		}
		else
		{
			foundProtocolHandler = mProtocolCommandHandlers.find("*");
			if (foundProtocolHandler != mProtocolCommandHandlers.end())
			{
				foundProtocolHandler->second(inCommand);
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
	
	
	void	vanguardbot::log_in(const string &userName, const string &password, const string &channelName, const string &channelPassword)
	{
		mChannelName = channelName;
		mUserName = userName;
        mPassword = password;
		
		string passMsg("PASS ");
		passMsg.append(password);
		send_message(passMsg);
		string nickMsg("NICK ");
		nickMsg.append(userName);
		send_message(nickMsg);
		send_message("CAP REQ : twitch.tv/membership");
		send_message("CAP REQ : twitch.tv/tags");
		send_message("CAP REQ : twitch.tv/commands");
		string joinMsg("JOIN #");
		joinMsg.append(channelName);
		send_message(joinMsg);
		
		send_chat_message("Hi.");

#ifndef _WIN32
        twitch::init();

        twitch botConnection(mChannelName, mPassword);
        auto botUserID = botConnection.user_id();
        auto channelInfo = botConnection.channel_info(botUserID);

        cout << "title: " << channelInfo.mTitle << "\n"
            << "game: " << channelInfo.mGameName << " ("
            << channelInfo.mGameID << ")" << endl;

        twitch streamerConnection(mChannelName, channelPassword);
        auto streamerUserID = botConnection.user_id();

        // 7208 Myst IV: Revelation
        // Return to Monkey Island
        auto gameID = streamerConnection.game_id(streamerUserID, "Myst IV: Revelation");
        streamerConnection.set_game(streamerUserID, gameID);
        // This Atrus guy called! He needs our help again!
        // Atrus is not home right now, please call Saveedro
        streamerConnection.set_stream_title(streamerUserID, "This Atrus guy called! He needs our help again!");
#endif
	}
	
	
	void	vanguardbot::log_out()
	{
		if( mChannelName.empty() )
		{
			return; // We were never logged in.
		}
		
		string partMsg("PART #");
		partMsg.append(mChannelName);
		send_message(partMsg);
		
		send_message("QUIT :bot is shutting down.");
		
		mChannelName.erase();
	}
	
} /* namespace vanguard */
