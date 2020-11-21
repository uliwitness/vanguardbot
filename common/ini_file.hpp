#pragma once


#include <string>
#include <map>


namespace vanguard {
	
	using namespace std;
	
	class ini_file
	{
	public:
		ini_file(const string& inFilePath);
		~ini_file();
		
		string value_for_key(const string& inKey) { return mSettings[inKey]; }
		void		set_key_to_value(const string& inKey, const string& inValue) { mSettings[inKey] = inValue; }
		
		void		save();
		
	protected:
		string				mFilePath;
		bool				mModified = false;
		map<string, string>	mSettings;
	};
	
}
