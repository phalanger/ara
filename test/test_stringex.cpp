
#include "ara/stringext.h"
#include <iostream>

void test_stringext()
{
	std::string str1 = "hello";
	const std::string str2 = "hello world";

	auto ext1 = ara::strext(str1);
	auto ext2 = ara::strext(str2);

	for (auto ch : ara::strext(str1)) {
		std::cout << ch << std::endl;
	}
	for (auto ch : ara::strext(str2)) {
		std::cout << ch << std::endl;
	}

	auto re1 = ara::strext(str1).trim_left(" \t\r\n");
	auto re2 = ara::strext(str2).trim_left(" \t\r\n");
}
