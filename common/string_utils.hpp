//
//  string_utils.hpp
//  vanguardbot
//
//  Created by Uli Kusterer on 07.07.18.
//  Copyright © 2018 Uli Kusterer. All rights reserved.
//

#pragma once

#include <string>
#include <vector>

namespace vanguard {

	using namespace std;
	
	void replace_with_in(const string& pattern, const string& replacement, string &target);

	vector<string>	split_string_at(const string& inTarget, const string& splitter);

	string join_strings_with(const vector<string>& strings, const string& separator);

	string tolower(const string& inString);

}
