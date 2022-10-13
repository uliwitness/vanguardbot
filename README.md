# Vanguardbot

## What is it?

A Twitch chatbot that is supposed to be easy to manage, easy to update, flexible and helpful to improve social
interactions with your viewers.

## Current Features

- Unlimited number of quote pools
- Unlimited number of joinable queues
- Arbitrary names for "add quote" command
- Notifications about first-time chatters (per stream, and ever)
- Counters
- Credits / active users list
- Moderator-only commands
- Timers that trigger messages or commands

## Planned Features

- Command groups
- Commands for setting the game and stream title on Twitch based on presets
- Setup GUI for creating commands
- Working Windows support
- Built-in OAuth login support (at least for GUI)
- Launching executables (e.g. shell scripts)

## Operating Systems

Right now Vanguardbot works on **Linux** and **macOS**.

I started creating a Visual Studio Solution for Windows, but
can't get the entitlements to work (it can't access its files on disk). If anyone is more familiar with entitlements
under Windows, help would be appreciated.

## Syntax

If you're running the command line version of this bot (instead of a GUI), launch it with the syntax:

    ./vanguardbot <userName> oauth:<oauthToken> [<channel> [<commandsFolder>]]

Where

- `userName` - The name of the user as which the chatbot should run.
- `oauthToken` - An OAuth token to let Vanguardbot log in as the chatbot user and talk to Twitch chat (this is what will
  be sent as the IRC password, but is not the password that user uses to log into Twitch). You can get an OAuth token
  e.g. via https://twitchapps.com/tmi/
- `channel` - The name of the channel whose chat you want the bot to watch for commands and where you want it to post.
  If you leave away `channel`, `userName` will be used as the channel name too.
- `commandsFolder` - The folder containing one folder each with an `.ini` file for each command the bot should
  understand.

If you follow the usual convention of having a separate bot user (say, `jennys_bot`) and your own streamer user (say,
`jenny_strimmah`), then the channel would be `jenny_strimmah` while the `userName` would be `jennys_bot`, and the
`oauthToken` would be the one for `jennys_bot`.
 
## Commands

Commands are kept in a special folder. By default, this is `~/Application Support/vanguardbot/` on macOS,
on Linux `~/.config/vanguardbot/` is used. You can pass a `commandsFolder` to Vanguardbot on the command line to
override this default.

That folder contains a subfolder for each command, named after the command. Each folder contains an `info.ini` file
describing the command's settings. There is also a special subfolder named `data` which the commands can use to store
user data (for example, the quote command uses this to save the list of quotes in a text file named after the command,
e.g. `quote.txt`).

If you add the line `mustBeManagement=true` to a command, that command will only be usable by moderators and the
broadcaster themselves. Similarly, add a `mustBeSubscriber=true` line to let only subscribers use a command.

For example commands, see the `common/example_commands/` folder in this repository.

### Quote Command

A quote command makes the bot output a random line from a text file when used, and optionally lets viewers add new
quotes using a separate command. A quote command's `info.ini` file looks like this:

    type=quote
    filename=quotes.txt
    addcommand=addquote

Where you can replace `quotes.txt` with any other text file name. You can name the folder containing the ini file
whatever you'd like the command to be, and can change `addquote` to any other word you want as the command. If you leave
away the `addcommand` line completely, viewers will not be able to add quotes, the only way to add quotes will be
directly editing the `quotes.txt` file. The `type=quote` must always be this way, to tell Vanguardbot that this is a
command that should behave like a quote command.

For example, if you wanted to have a "!joke" command in your channel, you could create a `joke` folder and make
its `info.ini` file read:

    type=quote
    filename=jokes.txt
    addcommand=addjoke

Note that `mustBeManagement` and  `mustBeSubscriber` only apply to the main command. To specify that only subscribers
or moderators/broadcaster may use the add command, use `addCommandMustBeManagement=true` and
`addCommandMustBeSubscriber=true`.

### Name Pool / Queue

A name pool is a special form of the quote command. All names are kept in a text file. So for example, if you were doing
a raffle in your stream and you wanted a "!draw" command to draw winners, and a "!join" command for them to join the
raffle, you would create a `draw` folder containing the following `info.ini` file:

    type=quote
    filename=rafflepool.txt
    addcommand=join
    addcommandpattern=$1=$USERNAME

This is basically the same as the quote command above: It defines a list of quotes that will be written in
the `rafflepool.txt` file in the `data` folder. To randomly pick a quote (a winner) you can use the chat command `!draw`
. To add a quote (add a user), write `!addname`.

But since `!join` would just add an empty quote, we use `addcommandpattern`, which rewrites the command's parameters.
The pattern is a comma-separated list of statements that say which parameters that were given to the command by the
users, should be handed off as which parameters of the `addquote` command. `$` followed by a number stands for a
parameter given to the command (i.e. a word following the `!addname` word). `$USERNAME` is a special word that is
replaced with the name of the user who sent the command. Since `addquote` usually uses all its parameters as the text to
use as a quote, we just replace the first parameter with the user name.

If we wanted to e.g. let the users additionally specify which raffle win they would like by e.g. saying `!join game` to
join the raffle and get the computer game, instead of getting a copy of your new novel (`!join novel`), we could do this
by using the following `addcommandpattern` instead.

    addcommandpattern=$1=$USERNAME,$2=$1

This assigns the name of the user who issued the command as the first parameter, and whatever was given as the first
parameter (in our case `game` or `novel`) is added as the second. All other parameters are discarded. Note that you can
also write other text besides these `$`-words, so if you wanted, you could write this as:

    addcommandpattern=$1=$USERNAME - $1

which would get you a dash between the user name and the word they wrote after `!addname`. Or if you wanted users to
write multiple words, you could write:

    addcommandpattern=$1=$USERNAME - $_

This special `$_` word is replaced with _all_ parameters the user gave, not just the first one.
The following `$`-words are recognized in this list of parameters:

`$1`, `$2` etc. - First, second etc. parameter.
`$DATE` - The current date
`$TIME` - The current time
`$USERNAME` - The name of the user who sent the command
`$_` - All parameters given to this command.

As you can see, the quote command can be quite powerful in Vanguardbot, and has many uses beyond just collecting quotes.

### Counters

A counter is a pair of commands, one of which displays a count, the other allows you incrementing the count
by one. This is a fun way of letting your viewers interactively take part in your stream by keeping count of
something, like how often you say "um", how often you burp, sneeze, die in a difficult game, or how rarely that
happens. For example, if you wanted to create a death counter, you could create a command folder named
`howdead` and put the following `info.ini` file into it:

    type=counter
    filename=deathcounter.txt
    response=$CHANNELNAME has died $COUNT times.
    incrementcommand=dead
    incrementresponse=$CHANNELNAME is now at $COUNT deaths, my dear $USERNAME.

The important part here is the `type=counter` so Vanguardbot knows this command is a counter. The `filename` is the file
in the `data` folder in which the count will be kept between restarts. The `response` is the message that Vanguardbot
will display to tell the viewer about the counter's count.

The `incrementcommand` is the name of the command to use for incrementing the counter. In this case, that would
be `!dead`. The `incrementresponse` is a message Vanguardbot will print to confirm that the counter has been
incremented. You can write three placeholders in the responses:

`$COUNT` - The count of the counter (in the case of incrementresponse, after incrementing)
`$CHANNELNAME` - The name of this channel, and therefore the broadcaster of this stream.
`$USERNAME` - The name of the user who invoked the counter command.

To have multiple counters, use different `filename` and `incrementcommand` values, and folder names.

Note that `mustBeManagement` and  `mustBeSubscriber` only apply to the main command. To specify that only subscribers
or moderators/broadcaster may use the increment command, use `incrementCommandMustBeManagement=true` and
`incrementCommandMustBeSubscriber=true`.

### Timers

A timer is a command that you don't invoke by name, but instead it outputs a message periodically. A simple example
timer would be a hydration reminder command that reminds you to drink every half hour (30 minutes) in a `hydratetimer`
folder with an `info.ini` file like:

    type=timer
    intervalminutes=30
    message=Remember to drink regularly, dehydration is bad for your kidneys!

You could also set up a `hydrate` quote command that shows a random hydration message from a bunch of different messages
in a text file and use a timer to trigger that from your `hydratetimer` timer:

    type=timer
    intervalminutes=30
    message=!hydrate

The cool thing here is that timers know about commands and will not actually send a "!hydrate" message to Twitch chat.
It will just internally pretend that it received this message and show the output from the command (in this case
from `!hydrate`).

### User List / Credits

Vanguardbot keeps a `data/todayseenusers.txt` list of all users that have actively chatted, one username per line, that
you can use to e.g. feed a credits text overlay at the end of your stream. This file is emptied whenever you start
Vanguardbot for the first time. It also keeps a `data/seenusers.txt` file, which does not get cleared and collects the
names of all users that have chatted whenever you had Vanguardbot running. You can use this to find out when a new user
chats for the first time.

## Building

To build Vanguardbot:

On Linux, use CMake or just open the repository folder with CLion.
On macOS, use the `vanguardbot.xcodeproj`.
On Windows, use the `vanguardbot_win.sln`.