#include <iostream>
#include "vanguardbot.hpp"

using namespace std;
using namespace vanguard;

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
        const char *configDir = getenv("XDG_CONFIG_HOME");
        if (!configDir || configDir[0] == 0) {
            configDir = getenv("HOME");
            if (!configDir || configDir[0] == 0) {
                cerr << "Can't find directory for config files." << endl;
                return 1;
            } else {
                commandsFolder = string(configDir) + "/.config";
            }
        } else {
            commandsFolder = configDir;
        }
        commandsFolder += "/vanguardbot/";
    }

	vanguardbot	bot;
	bot.connect("irc.chat.twitch.tv", 6667, commandsFolder, [&bot, userName, password, channel]()
	{
		bot.log_in(userName, password, channel);
		bot.run();
	});

    return 0;
}

