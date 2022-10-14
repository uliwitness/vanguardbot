#include "vanguardbot.hpp"
#include <iostream>
#include <signal.h>

using namespace std;
using namespace vanguard;

vanguardbot	gBot;

static void sigterm_handler(int signum)
{
    gBot.set_keep_running(false);
}

int main(int argc, const char* *argv)
{
    if (argc < 3)
    {
        cerr << "Error: Parameters should be " << argv[0] << " <userName> oauth:<oauthToken> [<channel> <channelPassword> [<commandsFolder>]]" << endl;
        return 1;
    }

    std::string userName(argv[1]);
    std::string password(argv[2]);
    std::string channel;
    string channelPassword;
    std::string commandsFolder;
    if (argc > 3 && argv[3][0] != 0) {
        channel = argv[3];
    } else {
        channel = userName;
    }

    if (argc > 4 && argv[4][0] != 0) {
        channelPassword = argv[4];
    } else {
        channelPassword = password;
    }

    if (argc > 5 && argv[5][0] != 0) {
        commandsFolder = argv[5];
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

    struct sigaction action = {};
    action.sa_handler = sigterm_handler;
    sigaction(SIGTERM, &action, NULL);

    gBot.connect("irc.chat.twitch.tv", 6667, commandsFolder, [userName, password, channel, channelPassword]()
	{
		gBot.log_in(userName, password, channel, channelPassword);
		gBot.run();
	});

    return 0;
}

