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
	
	string chomp_until_separator_from_string(const string &separator, string& inTarget)
	{
		string result;
		
		auto pos = inTarget.find(separator);
		if( pos != string::npos )
		{
			result = inTarget.substr(0, pos);
			inTarget.erase(0, pos + separator.length());
		}
		
		return result;
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
	
	string join_strings_with(const vector<string>& strings, const string& separator)
	{
		string result;
		bool first = true;
		
		for( const string& str : strings )
		{
			if( !first )
			{
				result.append(separator);
			}
			else
			{
				first = false;
			}
			result.append(str);
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
