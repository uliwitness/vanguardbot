#include <iostream>
#include "vanguardbot.hpp"

using namespace std;


int main(int argc, const char* *argv)
{
	if (argc < 3)
	{
		cerr << "Error: Parameters should be " << argv[0] << " <userName> oauth:<oauthToken> [<channel> [<commandsFolder>]]" << endl;
		return 1;
	}

	std::string userName(argv[1]);
	std::string password(argv[2]);
	std::string channel;
    std::string commandsFolder;
    if (argc > 3 && argv[3][0] != 0)
    {
        channel = argv[3];
    }
    else
    {
        channel = userName;
    }
    if (argc > 4 && argv[4][0] != 0)
    {
        commandsFolder = argv[4];
    }

    if (commandsFolder.empty()) {
        const char *homeFolder = getenv("HOME");
        if (!homeFolder) {
            cerr << "Can't find directory for config files." << endl;
            return 1;
        } else {
            commandsFolder = homeFolder + "/Application Support/vanguardbot/";
        }
    }

	vanguardbot	bot("irc.chat.twitch.tv", 6667, commandsFolder, [&bot, userName, password, channel]()
	{
		bot.log_in(userName, password, channel);
		bot.run();
	});

    return 0;
}

