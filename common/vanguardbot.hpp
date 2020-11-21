#pragma once

#include "vanguardbot_base.hpp"
#include "string_utils.hpp"
#include <functional>
#include <vector>
#include <map>
#include <set>

namespace vanguard {
	
	using namespace std;
	
	struct irc_command
	{
		string				command;
		string				userName;
		vector<string>		params;
		string				prefix;
		string				tagsString;
		map<string,string> 	tags;
	};
	
	
	typedef function<void(const irc_command&)> irc_command_handler;
	typedef function<void(const string&)> seen_user_handler;

	
	class vanguardbot : public vanguardbot_base
	{
	public:
		vanguardbot() : vanguardbot_base() {}
		
		virtual void	connect(const string& inHostname, int inPortNumber, const string& inFolderPath, function<void()> inReadyToRunHandler);
		
		void	log_in(string userName, string password, string channelName);
		void	log_out();
		
		void	send_chat_message(string msg);
		
		/*! Bot commands are things like "!quote". or "!addquote What are you doing".
			Specify the name without the exclamation mark, specify "*" to be called
			for every bot command for which no handler exists. */
		void	add_bot_command_handler(const string& name, irc_command_handler handler) { mBotCommandHandlers[tolower(name)] = handler; };
		
		//! Protocol commands are IRC's internal commands, like PRIVMSG or PING.
		void	add_protocol_command_handler(string name, irc_command_handler handler) { mProtocolCommandHandlers[name] = handler; };
		
		/*! Add a handler to be called the first time a user is seen since
			the bot launched. This handler is only called when the
			mEverSeenUsersHandler is not called. */
		void	set_today_seen_user_handler(seen_user_handler handler) { mTodaySeenUserHandler = handler; }
		
		/*! Add a handler to be called the first time a user is seen.
			This list is persisted across restarts, so you get notified
			when new people come to your stream. */
		void	set_ever_seen_user_handler(seen_user_handler handler) { mEverSeenUserHandler = handler; }

	protected:
		void			load_one_command_folder(const string &inCommandFolder);
		
		virtual void	process_one_line(const string& currLine);
		virtual void	process_full_lines();
		
		virtual void	handle_command(const irc_command& inCommand);
		
		irc_command		apply_pattern_to_command(const string& pattern, const irc_command &inCommand);
		
		string								mCommandsFolderPath;
		string								mChannelName;
		string								mUserName;
		map<string, irc_command_handler>	mProtocolCommandHandlers;
		map<string, irc_command_handler>	mBotCommandHandlers;
		set<string>							mTodaySeenUsers;
		seen_user_handler					mTodaySeenUserHandler;
		set<string>							mEverSeenUsers;
		seen_user_handler					mEverSeenUserHandler;
	};
	
}
