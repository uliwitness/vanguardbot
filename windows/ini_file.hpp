#pragma once


#include <string>
#include <map>


class ini_file
{
public:
	ini_file(const std::string& inFilePath);
	~ini_file();

	std::string value_for_key(const std::string& inKey) { return mSettings[inKey]; }
	void		set_key_to_value(const std::string& inKey, const std::string& inValue) { mSettings[inKey] = inValue; }

	void		save();

protected:
	std::string							mFilePath;
	bool								mModified = false;
	std::map<std::string, std::string>	mSettings;
};

