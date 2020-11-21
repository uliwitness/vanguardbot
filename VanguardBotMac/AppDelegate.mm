//
//  AppDelegate.m
//  VanguardBotMac
//
//  Created by Uli Kusterer on 07.07.18.
//  Copyright © 2018 Uli Kusterer. All rights reserved.
//

#import "AppDelegate.h"
#import "vanguardbot.hpp"


using namespace vanguard;


@interface AppDelegate () <NSTableViewDataSource>
{
	vanguardbot mBot;
}

@property (weak) IBOutlet NSWindow *window;
@property (weak) IBOutlet NSTextField *userNameField;
@property (weak) IBOutlet NSTextField *passwordField;
@property (weak) IBOutlet NSTextField *channelNameField;
@property (weak) IBOutlet NSPathControl *commandsFolderField;
@property (weak) IBOutlet NSButton *connectButton;
@property (weak) IBOutlet NSProgressIndicator *progress;
@property (weak) IBOutlet NSTableView *eventsList;

@property (strong) NSMutableArray *events;

@end

@implementation AppDelegate

- (void) log: (NSString*)format, ...
{
	va_list list = {};
	va_start(list, format);
	NSString * message = [[NSString alloc] initWithFormat: format arguments: list];
	va_end(list);
	[self.events addObject: message];
	[self.eventsList noteNumberOfRowsChanged];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	self.events = [@[@"Launched."] mutableCopy];
	[self.eventsList reloadData];
	
	[self.progress startAnimation: nil];

	NSString *commandsFolder = [@"~/Library/Application Support/vanguardbot" stringByExpandingTildeInPath];
	NSURL *commandsFolderURL = [NSURL fileURLWithPath:commandsFolder];
	self.commandsFolderField.URL = commandsFolderURL;
	
	if( ![NSFileManager.defaultManager fileExistsAtPath: commandsFolder] )
	{
		[self log: @"Creating commands folder at \"%@\"", commandsFolder];
		NSURL * exampleCommandsURL = [NSBundle.mainBundle URLForResource: @"example_commands" withExtension: nil subdirectory: nil];
		[NSFileManager.defaultManager copyItemAtURL: exampleCommandsURL toURL: commandsFolderURL error: NULL];
	}
	
	NSString * userName = [NSUserDefaults.standardUserDefaults objectForKey: @"VGBUserName"];
	if( userName )
	{
		self.userNameField.stringValue = userName;
		
		NSDictionary * secItemInfo = @{
			(__bridge NSString *)kSecAttrService: @"Twitch.tv OAuth Token",
			(__bridge NSString *)kSecClass: (__bridge NSString *)kSecClassGenericPassword,
			(__bridge NSString *)kSecAttrAccount: userName,
			(__bridge NSString *)kSecReturnData: @YES,
			(__bridge NSString *)kSecMatchLimit: (__bridge NSString *)kSecMatchLimitOne
		};
		CFTypeRef outData = NULL;
		if (SecItemCopyMatching( (__bridge CFDictionaryRef) secItemInfo, &outData ) == noErr)
		{
			NSData * theData = (__bridge_transfer NSData*)outData;
			NSString *oauthToken = [[NSString alloc] initWithData:theData encoding:NSUTF8StringEncoding];
			if (oauthToken)
			{
				self.passwordField.stringValue = oauthToken;
			}
		}
	}
	
	NSString * channelName = [NSUserDefaults.standardUserDefaults objectForKey: @"VGBChannelName"];
	if( channelName )
	{
		self.channelNameField.stringValue = channelName;
	}
	
	[self.progress stopAnimation: nil];
}


- (void)applicationWillTerminate:(NSNotification *)aNotification
{
	[self.progress startAnimation: nil];
	[self log: @"Signing off..."];

	mBot.log_out();
	
	NSString * userName = self.userNameField.stringValue;
	[NSUserDefaults.standardUserDefaults setObject: userName forKey:@"VGBUserName"];
	
	if (userName.length > 0 && self.passwordField.stringValue.length > 0)
	{
		NSDictionary * secItemInfo = @{
			(__bridge NSString *)kSecAttrService: @"Twitch.tv OAuth Token",
			(__bridge NSString *)kSecClass: (__bridge NSString *)kSecClassGenericPassword,
			(__bridge NSString *)kSecAttrAccount: userName,
			(__bridge NSString *)kSecValueData: [self.passwordField.stringValue dataUsingEncoding: NSUTF8StringEncoding]
		};
		SecItemDelete( (__bridge CFDictionaryRef) secItemInfo );
		SecItemAdd( (__bridge CFDictionaryRef) secItemInfo, NULL );
	}
	
	NSString * channelName = self.channelNameField.stringValue;
	[NSUserDefaults.standardUserDefaults setObject: channelName forKey:@"VGBChannelName"];

	self.connectButton.enabled = YES;
	[self log: @"Done."];
	[self.progress stopAnimation: nil];
}

- (IBAction)connectToServer:(id)sender
{
	[self log: @"Connecting..."];
	[self.progress startAnimation: nil];

	NSString *commandsFolder = self.commandsFolderField.URL.path;
	
	mBot.set_ever_seen_user_handler([self](const string& userName)
									{
		NSString* userNameObjC = [NSString stringWithUTF8String: userName.c_str()];
		[self log: @"NEW USER! %@", userNameObjC];
	});
	mBot.set_today_seen_user_handler([self](const string& userName)
									{
		NSString* userNameObjC = [NSString stringWithUTF8String: userName.c_str()];
		[self log: @"RETURNING USER! %@", userNameObjC];
	});
	mBot.connect("irc.chat.twitch.tv", 6667, commandsFolder.fileSystemRepresentation, [self]()
				 {
		NSString *userName = self.userNameField.stringValue;
		NSString *password = self.passwordField.stringValue;
		NSString *channelName = self.channelNameField.stringValue;
		mBot.log_in(userName.UTF8String, password.UTF8String, channelName.UTF8String);
		mBot.run();
		
		[self log: @"Connected."];
		self.connectButton.enabled = NO;
		[self.progress stopAnimation: nil];
	});
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
	return self.events.count;
}

- (nullable id)tableView:(NSTableView *)tableView objectValueForTableColumn:(nullable NSTableColumn *)tableColumn row:(NSInteger)row
{
	return self.events[row];
}

@end
