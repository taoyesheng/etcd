#pragma once
#include<string>
#include"xcore_define.h"

using namespace std;

namespace xcore
{
	///////////////////////////////////////////////////////////////////////////////
	// class XStrUtil
	///////////////////////////////////////////////////////////////////////////////
	class XStrUtil
	{
	public:
		// 去除字符串头(或尾)中在字符集中指定的字符
		static string& chop_head(string& strSrc, const char* pcszCharSet = " \t\r\n");
		static string& chop_tail(string& strSrc, const char* pcszCharSet = " \t\r\n");
		static string& chop(string& strSrc, const char* pcszCharSet = " \t\r\n"); // 去除头和尾中指定字符
		static string& chop_all(string& strSrc, const char* pcszCharSet = " \t\r\n"); // 去除字符串中所有指定字符
	};
}

