#include "stdafx.h"
#include "vanguardbot_base.hpp"
#include <iostream>


#pragma comment(lib, "Ws2_32.lib")


using namespace std;


vanguardbot_base::vanguardbot_base(std::string inHostname, int inPortNumber)
{
	WSADATA wsadata;

	int error = WSAStartup(0x0202, &wsadata);
	if (error)
	{
		cerr << "Error: Couldn't start up WinSock (" << error << ")" << endl;
	}

	// Did we get the right Winsock version?
	if (wsadata.wVersion != 0x0202)
	{
		cerr << "Error: Didn't get desired WinSock version." << endl;
		WSACleanup();
		return;
	}

	// Resolve host name into IP:
	hostent* hostname = gethostbyname(inHostname.c_str());
	if (!hostname)
	{
		int dwError = WSAGetLastError();
		if (dwError == WSAHOST_NOT_FOUND)
		{
			cout << "Host not found" << endl;
		}
		else if (dwError == WSANO_DATA)
		{
			cout << "No data record found" << endl;
		}
		else
		{
			cout << "Function failed with error: " << dwError << endl;
		}

		WSACleanup();
		return;
	}
	std::string IPAddress(inet_ntoa(**(in_addr**)hostname->h_addr_list));

	//Fill out the information needed to initialize a socket…
	SOCKADDR_IN target; //Socket address information

	target.sin_family = AF_INET;
	target.sin_port = htons(inPortNumber);
	target.sin_addr.s_addr = inet_addr(IPAddress.c_str());

	mSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (mSocket == INVALID_SOCKET)
	{
		cerr << "Error: Couldn't create socket." << endl;
		WSACleanup();
		return;
	}

	if (connect(mSocket, (SOCKADDR *)&target, sizeof(target)) == SOCKET_ERROR)
	{
		cerr << "Error: Couldn't connect to server." << endl;
		mSocket = INVALID_SOCKET;
		WSACleanup();
		return;
	}
}


vanguardbot_base::~vanguardbot_base()
{
	if (mSocket != INVALID_SOCKET)
	{
		closesocket(mSocket);

		WSACleanup();
	}
}


void vanguardbot_base::send_message(std::string inMessage)
{
	cout << "Sending: " << inMessage << endl;
	std::string messageWithLineBreak(inMessage);
	messageWithLineBreak.append("\r\n");

	int result = send(mSocket, messageWithLineBreak.c_str(), (int)messageWithLineBreak.length(), 0);
	if (result == SOCKET_ERROR)
	{
		cerr << "send failed: " << WSAGetLastError() << endl;
	}
}


void	vanguardbot_base::run()
{
	if (mSocket == INVALID_SOCKET)
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

