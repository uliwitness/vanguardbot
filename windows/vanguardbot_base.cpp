#include "vanguardbot_base.hpp"
#include <iostream>


using namespace Platform;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::System;
using namespace Windows::Networking::Sockets;
using namespace Concurrency;

using namespace std;


vanguardbot_base::vanguardbot_base(std::string inHostname, int inPortNumber, std::function<void()> inReadyToRunHandler)
{
	mSocket = ref new Windows::Networking::Sockets::StreamSocket();

	mSocket->Control->KeepAlive = false;

	create_task(mSocket->ConnectAsync(
		ref new Windows::Networking::HostName(StringFromStdString(inHostname)),
		StringFromStdString(to_string(inPortNumber)),
		SocketProtectionLevel::PlainSocket)).then([this, inReadyToRunHandler](task<void> previousTask)
	{
		try
		{
			previousTask.get(); // Trigger exceptions from tasks up to this point.
			
			cout << "Connected." << endl;

			mDataWriter = ref new Windows::Storage::Streams::DataWriter(mSocket->OutputStream);
			mDataReader = ref new Windows::Storage::Streams::DataReader(mSocket->InputStream);

			inReadyToRunHandler();
		}
		catch (Exception^ exception)
		{
			cout << "Couldn't connect: " << StdStringFromString(exception->Message) << endl;
		}
	});
}


vanguardbot_base::~vanguardbot_base()
{
	if (mSocket)
	{
		delete mSocket;
		mSocket = nullptr;
	}
}


void vanguardbot_base::send_message(std::string inMessage)
{
	cout << "Sending: " << inMessage << endl;
	std::string messageWithLineBreak(inMessage);
	messageWithLineBreak.append("\r\n");

	mDataWriter->WriteString(StringFromStdString(messageWithLineBreak));

	// Write the locally buffered data to the network.
	create_task(mDataWriter->StoreAsync()).then([this, inMessage](task<unsigned int> writeTask)
	{
		try
		{
			writeTask.get(); // Trigger exceptions, if any.

			cout << "Sent: " << inMessage.c_str() << endl;
		}
		catch (Exception^ exception)
		{
			cout << "Failed to send " << inMessage.c_str() << ": " << StdStringFromString(exception->Message) << endl;
		}
	});
}


void	vanguardbot_base::read_once()
{
	unsigned int amountToRead = max(mDataReader->UnconsumedBufferLength, 1U);
	create_task(mDataReader->LoadAsync(amountToRead)).then([this](unsigned int size)
	{
		if (size == 0)
		{
			// The underlying socket was closed before we were able to read the whole data.
			cancel_current_task();
		}

		string newData = StdStringFromString(mDataReader->ReadString(size));
		mMessageBuffer.append(newData);

		process_full_lines();
	}).then([this](task<void> previousTask)
	{
		try
		{
			// Try getting all exceptions from the continuation chain above this point.
			previousTask.get();

			// Everything went ok, so try to receive another string. The receive will continue until the stream is
			// broken (i.e. peer closed the socket).
			read_once();
		}
		catch (Exception^ exception)
		{
			cout << "Couldn't read from stream: " << StdStringFromString(exception->Message) << endl;

			// Explicitly close the socket.
			delete mSocket;
			mSocket = nullptr;
		}
		catch (task_canceled&)
		{
			// Do not print anything here - this will usually happen because user closed the client socket.

			// Explicitly close the socket.
			delete mSocket;
			mSocket = nullptr;
		}
	});
}


void	vanguardbot_base::run()
{
	if (mSocket == nullptr)
	{
		return;
	}

	mKeepRunning = true;

	read_once();
}

