#include "vanguardstringutils.hpp"
#include <codecvt>


using namespace Platform;
using namespace std;


Platform::String^ StringFromStdString(const string& inputString)
{
	wstring_convert<codecvt_utf8<wchar_t>> converter;
	wstring intermediateForm = converter.from_bytes(inputString);
	return ref new String(intermediateForm.c_str());
}


string StdStringFromString(String ^inputString)
{
	wstring_convert<codecvt_utf8<wchar_t>> converter;
	wstring intermediateForm(inputString->Data());
	return converter.to_bytes(intermediateForm);
}