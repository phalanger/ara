

#include "3rd/Catch/single_include/catch.hpp"

#include "ara/filesys.h"
#include "ara/error.h"

TEST_CASE("filesys", "[base]") {

	SECTION("path") {
#ifdef ARA_WIN32_VC_VER
		REQUIRE(ara::file_sys::to_path(std::string("C:\\abcd")) == "C:\\abcd\\");

		REQUIRE(ara::file_sys::join_to_path(std::string("C:\\abcd"), std::string("def")) == "C:\\abcd\\def\\");
		REQUIRE(ara::file_sys::join_to_path(std::string("C:\\abcd\\"), std::string("def")) == "C:\\abcd\\def\\");
		REQUIRE(ara::file_sys::join_to_path(std::string("C:\\abcd\\"), std::string("\\def")) == "C:\\abcd\\def\\");
		REQUIRE(ara::file_sys::join_to_path(std::string("C:\\abcd\\"), std::string("\\def\\")) == "C:\\abcd\\def\\");
		REQUIRE(ara::file_sys::join_to_path(std::string("C:\\abcd"), std::string("\\def\\")) == "C:\\abcd\\def\\");

		REQUIRE(ara::file_sys::join_to_path(std::string("C:\\abcd"), std::string("def"), std::string("ghi")) == "C:\\abcd\\def\\ghi\\");
		REQUIRE(ara::file_sys::join_to_path(std::wstring(L"C:\\abcd"), std::wstring(L"def"), std::wstring(L"ghi")) == L"C:\\abcd\\def\\ghi\\");

		REQUIRE(ara::file_sys::join_to_file(std::string("C:\\abcd"), std::string("def")) == "C:\\abcd\\def");
		REQUIRE(ara::file_sys::join_to_file(std::string("C:\\abcd\\"), std::string("def")) == "C:\\abcd\\def");
		REQUIRE(ara::file_sys::join_to_file(std::string("C:\\abcd\\"), std::string("\\def")) == "C:\\abcd\\def");
		REQUIRE(ara::file_sys::join_to_file(std::string("C:\\abcd"), std::string("\\def")) == "C:\\abcd\\def");

		REQUIRE(ara::file_sys::join_to_file(std::string("C:\\abcd"), std::string("\\def"), std::string("\\ghi")) == "C:\\abcd\\def\\ghi");
		REQUIRE(ara::file_sys::join_to_file(std::string("C:\\abcd"), std::string("\\def\\ghi"), std::string("\\jkl")) == "C:\\abcd\\def\\ghi\\jkl");

		REQUIRE(ara::file_sys::fix_path(std::string("C:\\\\abcd\\..\\.\\def\\..\\hij\\")) == "C:\\hij\\");
		REQUIRE(ara::file_sys::fix_path(std::string("C:\\abcd\\khg\\..\\.\\def\\..\\hij\\")) == "C:\\abcd\\hij\\");
		REQUIRE(ara::file_sys::fix_path(std::string("C:\\abcd\\..\\..\\def\\..\\hij\\")) == "\\hij\\");
		REQUIRE(ara::file_sys::fix_path(std::string("C:\\abcd\\\\..\\..\\def\\..\\..\\hij\\")) == "\\hij\\");
#endif

		REQUIRE(ara::file_sys::to_path(std::string("/abcd")) == "/abcd/");

		REQUIRE(ara::file_sys::join_to_path(std::string("/abcd"), std::string("def")) == "/abcd/def/");
		REQUIRE(ara::file_sys::join_to_path(std::string("/abcd/"), std::string("def")) == "/abcd/def/");
		REQUIRE(ara::file_sys::join_to_path(std::string("/abcd/"), std::string("/def")) == "/abcd/def/");
		REQUIRE(ara::file_sys::join_to_path(std::string("/abcd/"), std::string("/def/")) == "/abcd/def/");
		REQUIRE(ara::file_sys::join_to_path(std::string("/abcd"), std::string("/def/")) == "/abcd/def/");

		REQUIRE(ara::file_sys::join_to_path(std::string("/abcd"), std::string("def"), std::string("ghi")) == "/abcd/def/ghi/");
		REQUIRE(ara::file_sys::join_to_path(std::wstring(L"/abcd"), std::wstring(L"def"), std::wstring(L"ghi")) == L"/abcd/def/ghi/");

		REQUIRE(ara::file_sys::join_to_file(std::string("/abcd"), std::string("def")) == "/abcd/def");
		REQUIRE(ara::file_sys::join_to_file(std::string("/abcd/"), std::string("def")) == "/abcd/def");
		REQUIRE(ara::file_sys::join_to_file(std::string("/abcd/"), std::string("/def")) == "/abcd/def");
		REQUIRE(ara::file_sys::join_to_file(std::string("/abcd"), std::string("/def")) == "/abcd/def");

		REQUIRE(ara::file_sys::join_to_file(std::string("/abcd"), std::string("/def"), std::string("/ghi")) == "/abcd/def/ghi");
		REQUIRE(ara::file_sys::join_to_file(std::string("/abcd"), std::string("/def/ghi"), std::string("/jkl")) == "/abcd/def/ghi/jkl");

		REQUIRE(ara::file_sys::fix_path(std::string("//abcd/.././def/../hij/")) == "/hij/");
		REQUIRE(ara::file_sys::fix_path(std::string("/abcd/khg/.././def/../hij/")) == "/abcd/hij/");
		REQUIRE(ara::file_sys::fix_path(std::string("/abcd/../../def/../hij/")) == "/hij/");
		REQUIRE(ara::file_sys::fix_path(std::string("//abcd//../../def/../../hij/")) == "/hij/");

		REQUIRE(ara::file_sys::fix_path(std::wstring(L"//abcd/.././def/../hij/")) == L"/hij/");
		REQUIRE(ara::file_sys::fix_path(std::wstring(L"/abcd/khg/.././def/../hij/")) == L"/abcd/hij/");
		REQUIRE(ara::file_sys::fix_path(std::wstring(L"/abcd/../../def/../hij/")) == L"/hij/");
		REQUIRE(ara::file_sys::fix_path(std::wstring(L"//abcd//../../def/../../hij/")) == L"/hij/");
	}

	SECTION("stat") {
#ifdef ARA_WIN32_VER
		ara::file_adv_attr	attr;
		if (ara::file_sys::get_file_attr("C:\\Windows", attr)) {
			REQUIRE( attr.is_dir() );
		}
#endif
	}

	SECTION("path splitor") {
		std::wstring	wstrPath = L"C:\\Windows\\abc\\def";
		size_t i = 0;
		for (auto it : ara::file_sys::split_path(wstrPath)) {

			if (i == 0)
				REQUIRE(it == L"C:");
			else if (i == 1)
				REQUIRE(it == L"Windows");
			else if (i == 2)
				REQUIRE(it == L"abc");
			else if (i == 3)
				REQUIRE(it == L"def");
			++i;
		}
	}

	SECTION("scandir") {
		std::vector<std::string>		vectPathName;

#ifdef ARA_WIN32_VER
		for (auto it : ara::scan_dir("C:\\")) {
#else
		for (auto it : ara::scan_dir("/home")) {
#endif
			vectPathName.push_back(it);
		}
		REQUIRE(vectPathName.size() != 0);
	}

	SECTION("rwafile") {
		ara::raw_file	rf;
#ifdef ARA_WIN32_VER
		std::string strFile = "D:\\test.txt";
#else
		std::string strFile = "/tm/123.txt";
#endif

		ara::file_sys::unlink(strFile);

		if (!rf.open(strFile).create().random().read_write().done())
			FAIL("Open file " << strFile << " fail:" << ara::error::info());

		REQUIRE(rf.write("Hello", 5) == 5);
		REQUIRE(rf.tell() == 5);
		REQUIRE(rf.seek(2, std::ios::beg) == 2);

		char buf[10];
		REQUIRE(rf.read(buf, 3) == 3);
		REQUIRE(buf[0] == 'l');
		REQUIRE(buf[1] == 'l');
		REQUIRE(buf[2] == 'o');

		ara::file_sys::unlink(strFile);
	}
}
