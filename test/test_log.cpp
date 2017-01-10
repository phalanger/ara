
#include "3rd/Catch/single_include/catch.hpp"

#include "ara/log.h"
#include "ara/internal/log_appender_stdstream.h"

#include <sstream>

TEST_CASE("log", "[base]") {

	SECTION("simple log") {
		std::stringstream	s;

		ara::log::register_appender(std::make_shared<ara::log::appender_stdstream>(s));

		ara::glog(ara::log::info).printfln("Hello %d", 123);

		std::string strLog = s.str();

		REQUIRE(strLog.find("T:") == 0);
		REQUIRE(strLog.find("Hello 123") != std::string::npos);
	}

	SECTION("log 2") {

		std::stringstream	s;

		auto & root = ara::log::get_logger();
		root.set_appender(std::make_shared<ara::log::appender_stdstream>(s));

		LOG_INFO().printf("Hello %d", 124) << std::endl;

		std::string strLog = s.str();

		REQUIRE(strLog.find("T:") == 0);
		REQUIRE(strLog.find("Hello 124") != std::string::npos);

		size_t pos = strLog.size();
		LOG_INFO("App.log").printfln("%u", 8765);
		strLog = s.str();
		REQUIRE(strLog.find("T:", pos) == pos);
		REQUIRE(strLog.find("8765") != std::string::npos);
		REQUIRE(strLog.find("App.log") != std::string::npos);

		pos = strLog.size();
		LOG_DEBUG("App2.log").printfln("%u", 8745);
		strLog = s.str();
		REQUIRE(strLog.size() == pos);

		root.set_level(ara::log::debug);
		LOG_DEBUG("App2.log").printfln("%u", 8735);
		strLog = s.str();
		REQUIRE(strLog.find("T:", pos) == pos);
	}
}
