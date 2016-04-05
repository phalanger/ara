
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test_suite.hpp>
#include "ara/filesys.h"

#include <iostream>

BOOST_AUTO_TEST_SUITE(filesys)

BOOST_AUTO_TEST_CASE(filesys_base)
{
#ifdef ARA_WIN32_VC_VER
	BOOST_REQUIRE_EQUAL(ara::file_sys::to_path(std::string("C:\\abcd")), "C:\\abcd\\");

	BOOST_REQUIRE_EQUAL(ara::file_sys::join_to_path(std::string("C:\\abcd"), std::string("def")), "C:\\abcd\\def\\");
	BOOST_REQUIRE_EQUAL(ara::file_sys::join_to_path(std::string("C:\\abcd\\"), std::string("def")), "C:\\abcd\\def\\");
	BOOST_REQUIRE_EQUAL(ara::file_sys::join_to_path(std::string("C:\\abcd\\"), std::string("\\def")), "C:\\abcd\\def\\");
	BOOST_REQUIRE_EQUAL(ara::file_sys::join_to_path(std::string("C:\\abcd\\"), std::string("\\def\\")), "C:\\abcd\\def\\");
	BOOST_REQUIRE_EQUAL(ara::file_sys::join_to_path(std::string("C:\\abcd"), std::string("\\def\\")), "C:\\abcd\\def\\");

	BOOST_REQUIRE_EQUAL(ara::file_sys::join_to_file(std::string("C:\\abcd"), std::string("def")), "C:\\abcd\\def");
	BOOST_REQUIRE_EQUAL(ara::file_sys::join_to_file(std::string("C:\\abcd\\"), std::string("def")), "C:\\abcd\\def");
	BOOST_REQUIRE_EQUAL(ara::file_sys::join_to_file(std::string("C:\\abcd\\"), std::string("\\def")), "C:\\abcd\\def");
	BOOST_REQUIRE_EQUAL(ara::file_sys::join_to_file(std::string("C:\\abcd"), std::string("\\def")), "C:\\abcd\\def");

	BOOST_REQUIRE_EQUAL(ara::file_sys::fix_path(std::string("C:\\\\abcd\\..\\.\\def\\..\\hij\\")), "C:\\hij\\");
	BOOST_REQUIRE_EQUAL(ara::file_sys::fix_path(std::string("C:\\abcd\\khg\\..\\.\\def\\..\\hij\\")), "C:\\abcd\\hij\\");
	BOOST_REQUIRE_EQUAL(ara::file_sys::fix_path(std::string("C:\\abcd\\..\\..\\def\\..\\hij\\")), "\\hij\\");
	BOOST_REQUIRE_EQUAL(ara::file_sys::fix_path(std::string("C:\\abcd\\\\..\\..\\def\\..\\..\\hij\\")), "\\hij\\");
#endif

	BOOST_REQUIRE_EQUAL(ara::file_sys::to_path(std::string("/abcd")), "/abcd/");

	BOOST_REQUIRE_EQUAL(ara::file_sys::join_to_path(std::string("/abcd"), std::string("def")), "/abcd/def/");
	BOOST_REQUIRE_EQUAL(ara::file_sys::join_to_path(std::string("/abcd/"), std::string("def")), "/abcd/def/");
	BOOST_REQUIRE_EQUAL(ara::file_sys::join_to_path(std::string("/abcd/"), std::string("/def")), "/abcd/def/");
	BOOST_REQUIRE_EQUAL(ara::file_sys::join_to_path(std::string("/abcd/"), std::string("/def/")), "/abcd/def/");
	BOOST_REQUIRE_EQUAL(ara::file_sys::join_to_path(std::string("/abcd"), std::string("/def/")), "/abcd/def/");

	BOOST_REQUIRE_EQUAL(ara::file_sys::join_to_file(std::string("/abcd"), std::string("def")), "/abcd/def");
	BOOST_REQUIRE_EQUAL(ara::file_sys::join_to_file(std::string("/abcd/"), std::string("def")), "/abcd/def");
	BOOST_REQUIRE_EQUAL(ara::file_sys::join_to_file(std::string("/abcd/"), std::string("/def")), "/abcd/def");
	BOOST_REQUIRE_EQUAL(ara::file_sys::join_to_file(std::string("/abcd"), std::string("/def")), "/abcd/def");

	BOOST_REQUIRE_EQUAL(ara::file_sys::fix_path(std::string("//abcd/.././def/../hij/")), "/hij/");
	BOOST_REQUIRE_EQUAL(ara::file_sys::fix_path(std::string("/abcd/khg/.././def/../hij/")), "/abcd/hij/");
	BOOST_REQUIRE_EQUAL(ara::file_sys::fix_path(std::string("/abcd/../../def/../hij/")), "/hij/");
	BOOST_REQUIRE_EQUAL(ara::file_sys::fix_path(std::string("//abcd//../../def/../../hij/")), "/hij/");

	BOOST_REQUIRE(ara::file_sys::fix_path(std::wstring(L"//abcd/.././def/../hij/")) == L"/hij/");
	BOOST_REQUIRE(ara::file_sys::fix_path(std::wstring(L"/abcd/khg/.././def/../hij/")) == L"/abcd/hij/");
	BOOST_REQUIRE(ara::file_sys::fix_path(std::wstring(L"/abcd/../../def/../hij/")) == L"/hij/");
	BOOST_REQUIRE(ara::file_sys::fix_path(std::wstring(L"//abcd//../../def/../../hij/")) == L"/hij/");
}

BOOST_AUTO_TEST_CASE(filesys_scandir)
{
	std::vector<std::string>		vectPathName;

#ifdef ARA_WIN32_VER
	for (auto it : ara::scan_dir("C:\\")) {
#else
	for (auto it : ara::scan_dir("/home")) {
#endif
		vectPathName.push_back(it);
	}
	BOOST_REQUIRE_NE(vectPathName.size(), 0);
}

BOOST_AUTO_TEST_SUITE_END()
