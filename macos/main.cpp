#include <iostream>
#include "vanguardbot.hpp"

using namespace std;


int main(int argc, const char* *argv)
{
	if (argc < 4)
	{
		cerr << "Error: Parameters should be " << argv[0] << " <commandsFolder> <userName> oauth:<oauthToken> [<channel>]" << endl;
		return 1;
	}

	std::string commandsFolder(argv[1]);
	std::string userName(argv[2]);
	std::string password(argv[3]);
	std::string channel;
	if (argc > 4 && argv[4][0] != 0)
	{
		channel = argv[4];
	}
	else
	{
		channel = userName;
	}

	vanguardbot	bot("irc.chat.twitch.tv", 6667, commandsFolder, [&bot, userName, password, channel]()
	{
		bot.log_in(userName, password, channel);
		bot.run();
	});

    return 0;
}

