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

using namespace std;

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

MainPage::MainPage()
{
	InitializeComponent();
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

	seedField->Text = "Hmmmm...?";
	//std::cout << seedField->Text << std::endl;
}
