#include "vanguardbot_base.hpp"
#include <iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


using namespace std;


vanguardbot_base::vanguardbot_base(std::string inHostname, int inPortNumber)
{
	// Resolve host name into IP:
	hostent* hostname = gethostbyname(inHostname.c_str());
	if (!hostname)
	{
		cout << "Function failed with error: " << errno << endl;
		return;
	}
	std::string IPAddress(inet_ntoa(**(in_addr**)hostname->h_addr_list));

	//Fill out the information needed to initialize a socket
	sockaddr_in target; //Socket address information

	target.sin_family = AF_INET;
	target.sin_port = htons(inPortNumber);
	target.sin_addr.s_addr = inet_addr(IPAddress.c_str());

	mSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (mSocket == -1)
	{
		cerr << "Error: Couldn't create socket (" << errno << ")." << endl;
		return;
	}

	if (connect(mSocket, (sockaddr *)&target, sizeof(target)) == -1)
	{
		close(mSocket);
		cerr << "Error: Couldn't connect to server." << endl;
		mSocket = -1;
		return;
	}
}


vanguardbot_base::~vanguardbot_base()
{
	if (mSocket != -1)
	{
		close(mSocket);
	}
}


void vanguardbot_base::send_message(std::string inMessage)
{
	cout << "Sending: " << inMessage << endl;
	std::string messageWithLineBreak(inMessage);
	messageWithLineBreak.append("\r\n");

	ssize_t result = send(mSocket, messageWithLineBreak.c_str(), messageWithLineBreak.length(), 0);
	if (result == -1)
	{
		cerr << "send failed: " << errno << endl;
	}
}


void	vanguardbot_base::run()
{
	if (mSocket == -1)
	{
		return;
	}

	mKeepRunning = true;

	while (mKeepRunning)
	{
		char buffer[513] = {};
		recv(mSocket, buffer, sizeof(buffer) - 1, 0);

		mMessageBuffer.append(buffer);
		process_full_lines();
	}
}

