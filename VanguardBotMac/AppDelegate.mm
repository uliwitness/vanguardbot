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


@interface AppDelegate ()
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

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	[self.progress startAnimation: nil];

	NSString *commandsFolder = [@"~/Library/Application Support/vanguardbot" stringByExpandingTildeInPath];
	NSURL *commandsFolderURL = [NSURL fileURLWithPath:commandsFolder];
	self.commandsFolderField.URL = commandsFolderURL;
	
	if( ![NSFileManager.defaultManager fileExistsAtPath: commandsFolder] )
	{
		NSLog(@"Creating commands folder.");
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
	[self.progress stopAnimation: nil];
}

- (IBAction)connectToServer:(id)sender
{
	[self.progress startAnimation: nil];

	NSString *commandsFolder = self.commandsFolderField.URL.path;
	
	mBot.connect("irc.chat.twitch.tv", 6667, commandsFolder.fileSystemRepresentation, [self]()
				 {
		NSString *userName = self.userNameField.stringValue;
		NSString *password = self.passwordField.stringValue;
		NSString *channelName = self.channelNameField.stringValue;
		mBot.log_in(userName.UTF8String, password.UTF8String, channelName.UTF8String);
		mBot.run();
		
		self.connectButton.enabled = NO;
		[self.progress stopAnimation: nil];
	});
}

@end
