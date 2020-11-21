//
//  AppDelegate.m
//  VanguardBotMac
//
//  Created by Uli Kusterer on 07.07.18.
//  Copyright © 2018 Uli Kusterer. All rights reserved.
//

#import "AppDelegate.h"
#import "vanguardbot.hpp"
#import <UserNotifications/UserNotifications.h>

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
@property (weak) IBOutlet NSView *loginPanel;

@property (strong) NSMutableArray<NSAttributedString *> *events;

@end

@implementation AppDelegate

-(NSString *) currentTime
{
	static NSDateFormatter * sFormatter = nil;
	if( !sFormatter )
	{
		sFormatter = [NSDateFormatter new];
		sFormatter.dateStyle = NSDateFormatterNoStyle;
		sFormatter.timeStyle = NSDateFormatterShortStyle;
	}
	
	return [sFormatter stringFromDate: NSDate.date];
}

- (void) logLabel: (nullable NSString*)label format: (NSString*)format, ...
{
	va_list list = {};
	va_start(list, format);
	NSString * message = [[NSString alloc] initWithFormat: format arguments: list];
	va_end(list);
	NSString * timestamp = [NSString stringWithFormat: @"%@ ", self.currentTime];
	NSMutableAttributedString * fullMsg = [[NSMutableAttributedString alloc] initWithString: timestamp attributes: @{ NSForegroundColorAttributeName: NSColor.systemIndigoColor, NSFontAttributeName: [NSFont systemFontOfSize: 18.0] }];
	if (label != nil)
	{
		NSAttributedString * attrLabel = [[NSAttributedString alloc] initWithString: [label stringByAppendingString: @": "] attributes: @{ NSForegroundColorAttributeName: NSColor.controlTextColor, NSFontAttributeName: [NSFont boldSystemFontOfSize: 18.0] }];
		[fullMsg appendAttributedString: attrLabel];
	}
	NSAttributedString * attrMsg = [[NSAttributedString alloc] initWithString: message attributes: @{ NSForegroundColorAttributeName: NSColor.controlTextColor, NSFontAttributeName: [NSFont boldSystemFontOfSize: 18.0] }];
	[fullMsg appendAttributedString: attrMsg];

	[self.events addObject: fullMsg];
	[self.eventsList noteNumberOfRowsChanged];
	[self.eventsList scrollRowToVisible: self.events.count -1];
	[self.eventsList.window display];
}

- (void) logMinor: (NSString*)format, ...
{
	va_list list = {};
	va_start(list, format);
	NSString * message = [[NSString alloc] initWithFormat: format arguments: list];
	va_end(list);
	NSString * timestamp = [NSString stringWithFormat: @"%@ ", self.currentTime];
	NSMutableAttributedString * fullMsg = [[NSMutableAttributedString alloc] initWithString: timestamp attributes: @{ NSForegroundColorAttributeName: NSColor.systemGrayColor, NSFontAttributeName: [NSFont systemFontOfSize: 18.0] }];
	NSAttributedString * attrMsg = [[NSAttributedString alloc] initWithString: message attributes: @{ NSForegroundColorAttributeName: NSColor.systemGrayColor, NSFontAttributeName: [NSFont systemFontOfSize: 18.0] }];
	[fullMsg appendAttributedString: attrMsg];
	[self.events addObject: fullMsg];
	[self.eventsList noteNumberOfRowsChanged];
	[self.eventsList scrollRowToVisible: self.events.count -1];
	[self.eventsList.window display];
}

- (void) logNotice: (NSString*)format, ...
{
	va_list list = {};
	va_start(list, format);
	NSString * message = [[NSString alloc] initWithFormat: format arguments: list];
	va_end(list);
	NSString * timestamp = [NSString stringWithFormat: @"%@ ", self.currentTime];
	NSMutableAttributedString * fullMsg = [[NSMutableAttributedString alloc] initWithString: timestamp attributes: @{ NSForegroundColorAttributeName: NSColor.systemGrayColor, NSFontAttributeName: [NSFont systemFontOfSize: 18.0] }];
	NSFont * font = [NSFontManager.sharedFontManager convertFont: [NSFont systemFontOfSize: 18.0] toHaveTrait: NSItalicFontMask];
	NSAttributedString * attrMsg = [[NSAttributedString alloc] initWithString: message attributes: @{ NSForegroundColorAttributeName: NSColor.systemGrayColor, NSFontAttributeName: font }];
	[fullMsg appendAttributedString: attrMsg];
	[self.events addObject: fullMsg];
	[self.eventsList noteNumberOfRowsChanged];
	[self.eventsList scrollRowToVisible: self.events.count -1];
	[self.eventsList.window display];
}

- (void) logUser: (nullable NSString*)label event: (NSString*)format, ...
{
	va_list list = {};
	va_start(list, format);
	NSString * message = [[NSString alloc] initWithFormat: format arguments: list];
	va_end(list);
	NSString * timestamp = [NSString stringWithFormat: @"%@ ", self.currentTime];
	NSMutableAttributedString * fullMsg = [[NSMutableAttributedString alloc] initWithString: timestamp attributes: @{ NSForegroundColorAttributeName: NSColor.systemIndigoColor, NSFontAttributeName: [NSFont systemFontOfSize: 18.0] }];
	NSAttributedString * attrMsg = [[NSAttributedString alloc] initWithString: message attributes: @{ NSForegroundColorAttributeName: NSColor.controlAccentColor, NSFontAttributeName: [NSFont systemFontOfSize: 18.0] }];
	[fullMsg appendAttributedString: attrMsg];
	
	if (label != nil)
	{
		NSAttributedString * attrLabel = [[NSAttributedString alloc] initWithString: [@" " stringByAppendingString: label] attributes: @{ NSForegroundColorAttributeName: NSColor.controlAccentColor, NSFontAttributeName: [NSFont boldSystemFontOfSize: 18.0] }];
		[fullMsg appendAttributedString: attrLabel];
	}

	[self.events addObject: fullMsg];
	[self.eventsList noteNumberOfRowsChanged];
	[self.eventsList scrollRowToVisible: self.events.count -1];
	[self.eventsList.window display];
}


- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	self.events = [NSMutableArray new];
	[self.eventsList reloadData];
	
	[self.progress startAnimation: nil];

	NSString *commandsFolder = [@"~/Library/Application Support/vanguardbot" stringByExpandingTildeInPath];
	NSURL *commandsFolderURL = [NSURL fileURLWithPath:commandsFolder];
	self.commandsFolderField.URL = commandsFolderURL;
	
	if( ![NSFileManager.defaultManager fileExistsAtPath: commandsFolder] )
	{
		[self logMinor: @"Creating commands folder at \"%@\"", commandsFolder];
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
	
	[UNUserNotificationCenter.currentNotificationCenter requestAuthorizationWithOptions: UNAuthorizationOptionAlert | UNAuthorizationOptionBadge completionHandler:^(BOOL granted, NSError *__nullable error) { NSLog(@"%s: %@", granted ? "Granted" : "NOT GRANTED", error); }];
}


- (void)applicationWillTerminate:(NSNotification *)aNotification
{
	[self.loginPanel setHidden: NO];
	[self.progress startAnimation: nil];
	[self logMinor: @"Signing off..."];

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
	[self logMinor: @"Done."];
	[self.progress stopAnimation: nil];
}

- (IBAction)connectToServer:(id)sender
{
	[self.progress startAnimation: nil];

	NSString *commandsFolder = self.commandsFolderField.URL.path;
	
	mBot.set_ever_seen_user_handler([self](const string& userName)
									{
		NSString* userNameObjC = [NSString stringWithUTF8String: userName.c_str()];
		[self logUser: userNameObjC event: @"New user"];

		UNMutableNotificationContent * content = [UNMutableNotificationContent new];
		content.body = [NSString stringWithFormat: @"New user \"%@\"", userNameObjC];
		UNNotificationRequest * request = [UNNotificationRequest requestWithIdentifier: @"com.thevoidsoftware.userseen.ever" content: content trigger: nil];
		[UNUserNotificationCenter.currentNotificationCenter addNotificationRequest: request withCompletionHandler: nil];
	});
	mBot.set_today_seen_user_handler([self](const string& userName)
									{
		NSString* userNameObjC = [NSString stringWithUTF8String: userName.c_str()];
		[self logUser: userNameObjC event: @"Returning user"];

		UNMutableNotificationContent * content = [UNMutableNotificationContent new];
		content.body = [NSString stringWithFormat: @"Returning user \"%@\"", userNameObjC];
		UNNotificationRequest * request = [UNNotificationRequest requestWithIdentifier: @"com.thevoidsoftware.userseen.ever" content: content trigger: nil];
		[UNUserNotificationCenter.currentNotificationCenter addNotificationRequest: request withCompletionHandler: nil];
	});
	mBot.set_privmsg_handler([self](irc_command command)
							 {
		NSString * msg = [NSString stringWithUTF8String: command.params[1].c_str()];
		NSString * userNameObjC = [NSString stringWithUTF8String: command.userName.c_str()];
		string posterName(tolower(command.userName));
		if( posterName == tolower(mBot.userName())
		   || posterName == tolower(mBot.channelName()))
		{
			[self logMinor: @"%@: %@", userNameObjC, msg];
		}
		else
		{
			[self logLabel: userNameObjC format: @"%@", msg];
		}
	});
	mBot.set_notice_handler([self](const string& notice)
							 {
		NSString * msg = [NSString stringWithUTF8String: notice.c_str()];
		[self logNotice: @"%@", msg];
	});

	mBot.connect("irc.chat.twitch.tv", 6667, commandsFolder.fileSystemRepresentation, [self]()
				 {
		NSString *userName = self.userNameField.stringValue;
		NSString *password = self.passwordField.stringValue;
		NSString *channelName = self.channelNameField.stringValue;
		mBot.log_in(userName.UTF8String, password.UTF8String, channelName.UTF8String);
		mBot.run();
		
		self.connectButton.enabled = NO;
		[self.progress stopAnimation: nil];
		[self.loginPanel setHidden: YES];
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
