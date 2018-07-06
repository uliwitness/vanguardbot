//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "MainPage.xaml.h"
#include <iostream>
#include <string>


using namespace vanguardbot_win;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::System;
using namespace Windows::Security::Credentials;
using namespace Windows::Storage::Pickers;
using namespace Windows::Storage;

using namespace std;

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

MainPage::MainPage()
{
	InitializeComponent();

	PasswordCredential ^credential = nullptr;

	try {
		auto vault = ref new PasswordVault();

		IVectorView<PasswordCredential ^> ^credentialList = vault->FindAllByResource("vanguardbot_win");
		if (credentialList->Size > 0)
		{
			if (credentialList->Size == 1)
			{
				credential = credentialList->GetAt(0);
			}
		}
	}
	catch (Platform::COMException ^err)
	{
		credential = nullptr;
	}

	if (credential)
	{
		credential->RetrievePassword();
		userNameField->Text = credential->UserName;
		oauthTokenField->Text = credential->Password;
	}
}


void vanguardbot_win::MainPage::UsernamePasswordTextBox_TextChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e)
{
	auto vault = ref new Windows::Security::Credentials::PasswordVault();
	vault->Add(ref new Windows::Security::Credentials::PasswordCredential("vanguardbot_win", userNameField->Text, oauthTokenField->Text));
}


void vanguardbot_win::MainPage::TextBox_TextChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e)
{
	IAsyncOperation<IVector<AppDiagnosticInfo ^> ^> ^appInfo = AppDiagnosticInfo::RequestInfoAsync();
	auto asyncTask = concurrency::create_task(appInfo);
	asyncTask.then([](IVector<AppDiagnosticInfo ^> ^appInfoList)
	{
		for (AppDiagnosticInfo ^currApp : appInfoList)
		{
			cout << "App: " << StdStringFromString(currApp->AppInfo->DisplayInfo->DisplayName).c_str() << endl;
		}
	});
}


void vanguardbot_win::MainPage::Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	mVanguardBot = new vanguardbot("irc.chat.twitch.tv", 6667, StdStringFromString(commandsPathField->Text), [this]()
	{
		mVanguardBot->log_in(StdStringFromString(userNameField->Text), StdStringFromString(oauthTokenField->Text), StdStringFromString(channelNameField->Text));
		mVanguardBot->run();
	});
}


void vanguardbot_win::MainPage::FolderPicker_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	FolderPicker	^picker = ref new FolderPicker;
	picker->FileTypeFilter->Append( "*" );
	IAsyncOperation<StorageFolder ^> ^storageFolderOp = picker->PickSingleFolderAsync();
	auto asyncTask = concurrency::create_task(storageFolderOp);
	asyncTask.then([this](StorageFolder ^storageFolder)
	{
		cout << "Picked directory: " << StdStringFromString(storageFolder->Path) << endl;
		commandsPathField->Text = storageFolder->Path;
	});
}
