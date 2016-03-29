
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test_suite.hpp>
#include "ara/log.h"
#include "ara/internal/log_appender_stdstream.h"

#include <sstream>

BOOST_AUTO_TEST_SUITE(alog)

BOOST_AUTO_TEST_CASE(log_base)
{
	std::stringstream	s;

	auto root = ara::log::logger_mgr::get().get_root();
	root->set_appender(std::make_shared<ara::log::appender_stdstream<>>(s));

	ara::glog(ara::log::info).printfln("Hello %d", 123);

	std::string strLog = s.str();

	BOOST_REQUIRE_EQUAL(strLog.find("T:"), 0);
	BOOST_REQUIRE_NE(strLog.find("Hello 123"), std::string::npos);
}

BOOST_AUTO_TEST_CASE(log_base_2)
{
	std::stringstream	s;

	auto root = ara::log::logger_mgr::get().get_root();
	root->set_appender(std::make_shared<ara::log::appender_stdstream<>>(s));

	LOG_INFO().printf("Hello %d", 124) << std::endl;

	std::string strLog = s.str();

	BOOST_REQUIRE_EQUAL(strLog.find("T:"), 0);
	BOOST_REQUIRE_NE(strLog.find("Hello 124"), std::string::npos);

	size_t pos = strLog.size();
	LOG_INFO("App.log").printfln("%u", 8765);
	strLog = s.str();
	BOOST_REQUIRE_EQUAL(strLog.find("T:", pos), pos);
	BOOST_REQUIRE_NE(strLog.find("8765"), std::string::npos);
	BOOST_REQUIRE_NE(strLog.find("App.log"), std::string::npos);

	pos = strLog.size();
	LOG_DEBUG("App2.log").printfln("%u", 8745);
	strLog = s.str();
	BOOST_REQUIRE_EQUAL(strLog.size(), pos);

	root->set_level(ara::log::debug);
	LOG_DEBUG("App2.log").printfln("%u", 8735);
	strLog = s.str();
	BOOST_REQUIRE_EQUAL(strLog.find("T:", pos), pos);
}

BOOST_AUTO_TEST_SUITE_END()
