

#include "3rd/Catch2/catch.hpp"

#include "ara/filesys.h"
#include "ara/error.h"

static void	test_split_path(const std::string & full, const std::string & path, const std::string & file)
{
	std::string path1, file1;
	ara::file_sys::split_path(full, path1, file1);
	REQUIRE(path == path1);
	REQUIRE(file == file1);
}

TEST_CASE("filesys", "[base]") {

	SECTION("path") {
#ifdef ARA_WIN32_VC_VER
		REQUIRE(ara::file_sys::to_path(std::string("C:\\abcd")) == "C:\\abcd\\");
		REQUIRE(ara::file_sys::to_path<std::string>("C:\\abcd") == "C:\\abcd\\");

		REQUIRE(ara::file_sys::join_to_path(std::string("C:\\abcd"), std::string("def")) == "C:\\abcd\\def\\");
		REQUIRE(ara::file_sys::join_to_path<std::string>("C:\\abcd", "def") == "C:\\abcd\\def\\");
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
		REQUIRE(ara::file_sys::fix_path(std::string("C:\\abcd\\..\\..\\def\\..\\hij\\")) == "C:\\hij\\");
		REQUIRE(ara::file_sys::fix_path(std::string("C:\\abcd\\\\..\\..\\def\\..\\..\\hij\\")) == "C:\\hij\\");

		REQUIRE(ara::file_sys::is_path<std::string>("C:\\abcde\\"));
		REQUIRE_FALSE(ara::file_sys::is_path<std::string>("C:\\abcde"));

		test_split_path("C:\\123\\456.txt", "C:\\123\\", "456.txt");
		test_split_path("C:\\123\\456\\", "C:\\123\\", "456");
		test_split_path("C:\\123", "C:\\", "123");
		
#endif

		REQUIRE(ara::file_sys::to_path(std::string("/abcd")) == "/abcd/");
		REQUIRE(ara::file_sys::to_path<std::string>("/abcd") == "/abcd/");

		REQUIRE(ara::file_sys::join_to_path(std::string("/abcd"), std::string("def")) == "/abcd/def/");
		REQUIRE(ara::file_sys::join_to_path<std::string>("/abcd", "def") == "/abcd/def/");
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

		REQUIRE(ara::file_sys::is_path<std::string>("/abcde/"));
		REQUIRE_FALSE(ara::file_sys::is_path<std::string>("/abcde"));

		test_split_path("/abc/def.txt", "/abc/", "def.txt");
		test_split_path("/abc/def/", "/abc/", "def");
		test_split_path("/abc", "/", "abc");
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
#ifdef ARA_WIN32_VER
		std::wstring	wstrPath = L"C:\\Windows\\abc\\def";
#else
		std::wstring	wstrPath = L"/root/Windows/abc/def";
#endif
		size_t i = 0;

		for (auto it : ara::file_sys::split_path(wstrPath)) {

			if (i == 0)
#ifdef ARA_WIN32_VER
				REQUIRE(it == L"C:");
#else
				REQUIRE(it == L"root");
#endif
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

	SECTION("rawfile") {
		ara::raw_file	rf;

		std::string strFile;
		REQUIRE(ara::file_sys::get_temp_folder(strFile));
		strFile += ara::file_sys::path_slash();
		strFile += "123.txt";

		ara::file_sys::unlink(strFile);

		bool boOpenFile = rf.open(strFile).create().random().read_write().done();
		INFO("Open file " << strFile << " result:" << ara::error::info());
		REQUIRE(boOpenFile);

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

	SECTION("change_filetime") {

		std::string strFile;
		REQUIRE(ara::file_sys::get_temp_folder(strFile));
		strFile += ara::file_sys::path_slash();
		strFile += "text.txt";

		ara::raw_file::save_data_to_file(strFile, "Hello");

		ara::date_time t = ara::date_time::get_current();
		t.step(-1, 0, 0);
		bool boChangeTimeOK = ara::file_sys::update_file_time(strFile, t, t);
		REQUIRE(boChangeTimeOK);

		ara::file_adv_attr	attr;
		REQUIRE(ara::file_sys::get_file_attr(strFile, attr) == true);
		REQUIRE(attr.modify_time == t);
		REQUIRE(attr.access_time == t);
	}
}
