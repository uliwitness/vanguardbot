#include "stdafx.h"
#include "vanguardbot.hpp"
#include <iostream>


using namespace std;


void	vanguardbot::process_one_line(string currLine)
{
	mLineHandler(currLine);
}


void	vanguardbot::process_full_lines()
{
	while (true)
	{
		size_t lineBreakPos = mMessageBuffer.find("\r\n");
		if (lineBreakPos != string::npos)
		{
			string currLine(mMessageBuffer.substr(0, lineBreakPos));
			mMessageBuffer.erase(0, lineBreakPos + 2);

			process_one_line(currLine);
		}
		else
		{
			break;
		}
	}
}
