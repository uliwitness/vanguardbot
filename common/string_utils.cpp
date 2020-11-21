//
//  string_utils.cpp
//  vanguardbot
//
//  Created by Uli Kusterer on 07.07.18.
//  Copyright © 2018 Uli Kusterer. All rights reserved.
//

#include "string_utils.hpp"
#include <ctype.h>


namespace vanguard {

using namespace std;


void replace_with_in(const string& pattern, const string& replacement, string &target)
{
	size_t currPos = 0;
	while(currPos < target.length())
	{
		size_t foundPos = target.find(pattern, currPos);
		if (foundPos == string::npos)
		{
			return;
		}
		
		target.replace(foundPos, pattern.length(), replacement);
		currPos = foundPos + replacement.length();
	}
}


vector<string>	split_string_at(const string& inTarget, const string& splitter)
{
	vector<string> result;
	
	size_t currPos = 0;
	while (currPos < inTarget.length())
	{
		size_t separatorPos = inTarget.find(splitter, currPos);
		if (separatorPos == string::npos)
		{
			separatorPos = inTarget.length();
		}
		result.push_back(inTarget.substr(currPos, separatorPos - currPos));
		
		currPos = separatorPos + splitter.length();
	}
	
	return result;
}

string tolower(const string& inString)
{
	size_t x = 0;
	string result(inString.length(), ' ');
	for( char currChar : inString )
	{
		result[x++] = std::tolower(currChar); // TODO: Make Unicode-aware.
	}
	return result;
}

} /* namespace vanguard */
