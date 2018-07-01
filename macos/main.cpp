#include <iostream>
#include "vanguardbot.hpp"

using namespace std;


int main(int argc, const char* *argv)
{
	if (argc < 3)
	{
		cerr << "Error: Parameters should be vanguardbot.exe <userName> oauth:<oauthToken> [<channel>]" << endl;
		return 1;
	}

	std::string userName(argv[1]);
	std::string password(argv[2]);
	std::string channel;
	if (argc > 3 && argv[3][0] != 0)
	{
		channel = argv[3];
	}
	else
	{
		channel = userName;
	}

	vanguardbot	bot("irc.chat.twitch.tv", 6667);
	bot.log_in(userName, password, channel);
	bot.run();

    return 0;
}

