#include "ini_file.hpp"
#include <fstream>


using namespace std;

ini_file::ini_file(const string &inFilePath)
	: mFilePath(inFilePath)
{
	string currKey;
	string currValue;

	fstream file( inFilePath, std::ios::in );

	while (file.good())
	{
		bool isInKey = true;

		int currCh = file.get();
		switch (currCh)
		{
		case '=':
			isInKey = false;
			break;

		case '\r':
		case '\n':
			isInKey = true;
			if (!currKey.empty())
			{
				mSettings[currKey] = currValue;
				currKey.erase();
				currValue.erase();
			}
			break;

		case 0:
		case -1:
			break;

		default:
			if (isInKey)
			{
				currKey.append(1, currCh);
			}
			else
			{
				currValue.append(1, currCh);
			}
			break;
		}
	}
}

ini_file::~ini_file()
{
}

void ini_file::save()
{
	fstream file(mFilePath, std::ios::out);

	for (pair<string, string> currSetting : mSettings)
	{
		file << currSetting.first << "=" << currSetting.second << endl;
	}
}
