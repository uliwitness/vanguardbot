#include "vanguardbot_base.hpp"
#include <iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#import <Foundation/Foundation.h>


#define IRC_PROTOCOL_MESSAGE_LENGTH_LIMIT	512


using namespace std;


@interface VGBIVars : NSObject  <NSStreamDelegate>
{
	void (^_textCallback)(const char*);
}

@property NSInputStream * readStream;
@property NSOutputStream * writeStream;
@property NSMutableArray<NSTimer *>* timers;

-(instancetype) initWithTextCallback:(void(^)(const char*))textCallback hostname:(NSString *)hostname portNumber:(int)inPortNumber;

@end


namespace vanguard {
	
	void	vanguardbot_base::connect(const std::string& inHostname, int inPortNumber, const std::string& inFolderPath, std::function<void()> inReadyToRunHandler)
	{
		NSString *hostname = [NSString stringWithUTF8String: inHostname.c_str()];
		
		mIVars = [[VGBIVars alloc] initWithTextCallback: ^(const char* msg)
				  {
			this->mMessageBuffer.append(msg);
			this->process_full_lines();
		} hostname: hostname portNumber: inPortNumber];
		
		inReadyToRunHandler();
	}
	
	
	vanguardbot_base::~vanguardbot_base()
	{
		mIVars = nil;
	}
	
	
	void vanguardbot_base::send_message(std::string inMessage)
	{
		cout << "Sending: " << inMessage << endl;
		std::string messageWithLineBreak(inMessage);
		messageWithLineBreak.append("\r\n");
		
		NSInteger bytesWritten = [[mIVars writeStream] write: (const uint8_t *)messageWithLineBreak.c_str() maxLength: messageWithLineBreak.length()];
		if( bytesWritten != messageWithLineBreak.length() )
		{
			NSLog(@"Tried to write %ld bytes, only managed to write %zu", messageWithLineBreak.length(), (long) bytesWritten);
		}
	}
	
	void	vanguardbot_base::perform_after(chrono::minutes delay, bool repeat, function<void()> handler)
	{
		double interval = delay.count() * 60.0;
		[((VGBIVars*)mIVars).timers addObject: [NSTimer scheduledTimerWithTimeInterval: (NSTimeInterval) interval repeats: repeat != false block: ^(NSTimer * _Nonnull timer)
		{
			handler();
		}]];
	}

} /* namespace vanguard */

@implementation VGBIVars

-(instancetype) initWithTextCallback:(void(^)(const char*))textCallback hostname:(NSString *)hostname portNumber:(int)inPortNumber
{
	if (self = [super init] )
	{
		_textCallback = textCallback;
		self.timers = [NSMutableArray new];
		
		CFReadStreamRef readStream = NULL;
		CFWriteStreamRef writeStream = NULL;
		
		CFStreamCreatePairWithSocketToHost(kCFAllocatorDefault, (__bridge CFStringRef)hostname, inPortNumber, &readStream, &writeStream);
		
		self.readStream = (__bridge_transfer NSInputStream *)readStream;
		self.writeStream = (__bridge_transfer NSOutputStream *)writeStream;
		
		self.readStream.delegate = self;
		[self.readStream open];
		[self.readStream scheduleInRunLoop: [NSRunLoop currentRunLoop] forMode: NSDefaultRunLoopMode];
		[self.writeStream open];
		[self.writeStream scheduleInRunLoop: [NSRunLoop currentRunLoop] forMode: NSDefaultRunLoopMode];
	}
	return self;
}


-(void) dealloc
{
	[self.timers performSelector: @selector(invalidate)];
}


-(void) stream:(NSStream *)aStream handleEvent:(NSStreamEvent)eventCode
{
	if( eventCode == NSStreamEventHasBytesAvailable )
	{
		char currStr[IRC_PROTOCOL_MESSAGE_LENGTH_LIMIT + 1] = {};
		NSInteger bytesRead = [self.readStream read:(uint8_t *)currStr maxLength:IRC_PROTOCOL_MESSAGE_LENGTH_LIMIT];
		if( bytesRead < 0 )
			return;
		currStr[bytesRead] = 0;
		
		NSLog(@"Received %ld bytes: %s", (long)bytesRead, currStr);
		_textCallback(currStr);
	}
}

@end
