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

	struct bot_command_handler_entry {
		irc_command_handler handler;
		bool mustBeManagement;
		bool mustBeSubscriber;
	};
	
	
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
		void	add_bot_command_handler(const string& name, irc_command_handler handler, bool mustBeManagement, bool mustBeSubscriber) { mBotCommandHandlers[tolower(name)] = bot_command_handler_entry{ .handler = handler, .mustBeManagement = mustBeManagement, .mustBeSubscriber = mustBeSubscriber }; };
		
		/*! Protocol commands are IRC's internal commands, like PRIVMSG or PING.
			If you set this for PRIVMSG, it will replace the handling that calls
			the bot command handler.*/
		void	add_protocol_command_handler(string name, irc_command_handler handler) { mProtocolCommandHandlers[name] = handler; };
		
		/*! If a PRIVMSG command arrives that is not a bot command, it will be handed
			off to this handler. */
		void	set_privmsg_handler(irc_command_handler handler) { mPrivMsgHandler = handler; }
		
		/*! Add a handler to be called the first time a user is seen since
			the bot launched. This handler is only called when the
			mEverSeenUsersHandler is not called. */
		void	set_today_seen_user_handler(seen_user_handler handler) { mTodaySeenUserHandler = handler; }
		
		/*! Add a handler to be called the first time a user is seen.
			This list is persisted across restarts, so you get notified
			when new people come to your stream. */
		void	set_ever_seen_user_handler(seen_user_handler handler) { mEverSeenUserHandler = handler; }
		
		string 	userName() { return mUserName; }

	protected:
		void			load_one_command_folder(const string &inCommandFolder);
		
		virtual void	process_one_line(const string& currLine);
		virtual void	process_full_lines();
		
		virtual void	handle_command(const irc_command& inCommand);
		
		irc_command		apply_pattern_to_command(const string& pattern, const irc_command &inCommand);
		
		string									mCommandsFolderPath; //!< Folder containing commands and data.
		string									mChannelName; //!< Name of the stream channel.
		string									mUserName; //!< Name of the bot user.
		map<string, irc_command_handler>		mProtocolCommandHandlers;
		map<string, bot_command_handler_entry>	mBotCommandHandlers;
		set<string>								mTodaySeenUsers; //!< Usernames.
		seen_user_handler						mTodaySeenUserHandler;
		set<string>								mEverSeenUsers; //!< Usernames.
		seen_user_handler						mEverSeenUserHandler;
		map<string, map<string, string>>		mUserTags;	//!< Username -> tags map.
		map<string, string>						mRoomTags;	//!< Tag name -> value map.
		irc_command_handler						mPrivMsgHandler; //!< Handler we send your PrivMsg to if it wasn't a command.
	};
	
}
