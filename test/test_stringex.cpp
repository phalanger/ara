
#include "ara/stringext.h"
#include <iostream>
#include <cassert>

void test_stringext()
{
	std::string str1 = " hello\r\t";
	const std::string str2 = "\rhello world\n\t ";

	auto ext1 = ara::strext(str1);
	auto ext2 = ara::strext(str2);

	size_t i = 0;
	for (auto ch : ara::strext(str1)) {
		assert(ch == str1[i++]);
	}
	i = 0;
	for (auto ch : ara::strext(str2)) {
		assert(ch == str2[i++]);
	}

	auto re1 = ara::strext(str1).trim_left(" \t\r\n");
	assert(re1 == "hello\r\t");

	auto re2 = ara::strext(str2).trim_right(" \t\r\n");
	assert(re2 == "\rhello world");
}
