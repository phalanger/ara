
#include "3rd/Catch/single_include/catch.hpp"

#include "ara/session.h"

class data
{
public:
	data() {}
	data(int a, const char * s) : age_(a), name_(s) {}

	int		age_ = 0;
	std::string	name_;
};

TEST_CASE("session", "[base]") {

	using	MySession = ara::session<std::string, data>;

	MySession		sess(7);

	std::shared_ptr<const data> pReadData = sess.get_read("Hello");
	REQUIRE(pReadData == nullptr);

	pReadData = sess.get_read("Hello", ara::ses_clear_expire);
	REQUIRE(pReadData == nullptr);

	std::shared_ptr<data> pWriteData = sess.get_write("Hello");
	REQUIRE(pWriteData == nullptr);

	//Create a new item.
	pWriteData = sess.create("Hello", data());
	REQUIRE(pWriteData != nullptr);
	REQUIRE(pWriteData->age_ == 0);
	pWriteData->age_ = 100;

	//It will OVERWRITE the old value
	pWriteData = sess.create("Hello", std::make_shared<data>(11, "hello"));
	REQUIRE(pWriteData != nullptr);
	REQUIRE(pWriteData->age_ == 11);
	pWriteData->age_ = 100;

	//It will get the old item.
	pWriteData = sess.get_write_or_create("Hello");
	REQUIRE(pWriteData != nullptr);
	REQUIRE(pWriteData->age_ == 100);

	//It will create a new item.
	pWriteData = sess.get_write_or_create("Hello2");
	REQUIRE(pWriteData != nullptr);
	REQUIRE(pWriteData->age_ == 0);
	pWriteData->age_ = 101;

	pReadData = sess.get_read("Hello2");
	REQUIRE(pReadData != nullptr);
	REQUIRE(pReadData->age_ == 101);

	size_t nCount = 0;
	sess.navigate([&nCount](size_t nIndex, MySession::control_data & d)->int {
		++nCount;
		return ara::ses_nav_continue;
	});
	REQUIRE(nCount == 2);
}