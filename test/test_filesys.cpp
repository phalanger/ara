
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test_suite.hpp>

#include "ara/filesys.h"
#include "ara/error.h"

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

BOOST_AUTO_TEST_CASE(filesys_stat)
{
#ifdef ARA_WIN32_VER
	ara::file_adv_attr	attr;
	if (ara::file_sys::get_file_attr("C:\\Windows", attr)) {
		BOOST_REQUIRE(attr.is_dir());
	}
#endif
}

BOOST_AUTO_TEST_CASE(filesys_pathsplitor)
{
	std::wstring	wstrPath = L"C:\\Windows\\abc\\def";
	size_t i = 0;
	for (auto it : ara::file_sys::split_path(wstrPath)) {

		if (i == 0)
			BOOST_REQUIRE(it == L"C:");
		else if (i == 1)
			BOOST_REQUIRE(it == L"Windows");
		else if (i == 2)
			BOOST_REQUIRE(it == L"abc");
		else if (i == 3)
			BOOST_REQUIRE(it == L"def");

		++i;
	}
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

BOOST_AUTO_TEST_CASE(filesys_rawfile)
{
	ara::raw_file	rf;
#ifdef ARA_WIN32_VER
	std::string strFile = "D:\\test.txt";
#else
	std::string strFile = "/tm/123.txt";
#endif

	ara::file_sys::unlink(strFile);

	if (!rf.open(strFile).create().random().read_write().done())
	{
		std::cout << "Error:" << ara::error::info() << std::endl;
		BOOST_ASSERT(false);
	}
	BOOST_REQUIRE_EQUAL(rf.write("Hello", 5), 5);
	BOOST_REQUIRE_EQUAL(rf.tell(), 5);
	BOOST_REQUIRE_EQUAL(rf.seek(2, std::ios::beg), 2);

	char buf[10];
	BOOST_REQUIRE_EQUAL(rf.read(buf, 3), 3);
	BOOST_REQUIRE_EQUAL(buf[0], 'l');
	BOOST_REQUIRE_EQUAL(buf[1], 'l');
	BOOST_REQUIRE_EQUAL(buf[2], 'o');

	ara::file_sys::unlink(strFile);
}

BOOST_AUTO_TEST_SUITE_END()
