
#include "3rd/Catch/single_include/catch.hpp"

#include "ara/utf8.h"

template<class strSrc, class strTar>
static void check_convert1(const strSrc & s, const strTar & r)
{
	strTar s1 = ara::utf8::convert<strTar>(s);
	strTar s2 = ara::utf8::convert<strTar, ara::utf8::policy::exception_while_error>(s);
	strTar s3 = ara::utf8::convert<strTar, ara::utf8::policy::skip_error_fast>(s);

	REQUIRE(s1.size() == r.size());
	REQUIRE(s1 == r);
	REQUIRE(s2 == r);
	REQUIRE(s3 == r);
}

TEST_CASE("utf8", "[base]") {
	SECTION("base") {
		const char32_t	tar_32[] = {
			0x41, 0x42, 0x43,	//ABC
			0x4e2d, 0x6587,		//Chinese
			0x1f600, 0x1f601,	//Emoji
			0
		};
		const char16_t tar_16[] = {
			65, 66, 67,
			20013,25991,
			55357, 56832,
			55357, 56833,
			0
		};

		const wchar_t tar_w[] = {
			65, 66, 67,
			20013,25991,
			55357, 56832,
			55357, 56833,
			0
		};

		const unsigned char tar_a[] = {
			0x41, 0x42, 0x43,
			0xe4, 0xb8, 0xad, 0xe6, 0x96, 0x87,
			0xf0, 0x9f, 0x98, 0x80, 0xf0, 0x9f, 0x98, 0x81,
			0x0
		};

		std::u32string	str1_32 = tar_32;
		std::u16string	str1_16 = tar_16;
		std::wstring	str1_w = tar_w;
		std::string		str1_a = (const char *)tar_a;

		SECTION("check str32 ->")
		{
			SECTION("->32"){check_convert1(str1_32, str1_32);}
			SECTION("->16"){check_convert1(str1_32, str1_16);}
			SECTION("->w"){check_convert1(str1_32, str1_w);}
			SECTION("->a"){check_convert1(str1_32, str1_a);}
		}

		SECTION("check str16->")
		{
			check_convert1(str1_16, str1_32);
			check_convert1(str1_16, str1_16);
			check_convert1(str1_16, str1_w);
			check_convert1(str1_16, str1_a);
		}

		SECTION("check wstr->")
		{
			check_convert1(str1_w, str1_32);
			check_convert1(str1_w, str1_16);
			check_convert1(str1_w, str1_w);
			check_convert1(str1_w, str1_a);
		}

		SECTION("check astr->")
		{
			check_convert1(str1_a, str1_32);
			check_convert1(str1_a, str1_16);
			check_convert1(str1_a, str1_w);
			check_convert1(str1_a, str1_a);
		}
	}
}
