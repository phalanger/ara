
#include "3rd/Catch/single_include/catch.hpp"

#include "ara/threadext.h"

#include <sstream>

class TestAutoDel : public ara::auto_del_base
{
public:
	TestAutoDel() {}
protected:
	int	a_ = 100;
};

class TLocalData
{
public:
	TLocalData() {}

	int	get() const { return a_; }
	void set(int a) { a_ = a; }
protected:
	int a_ = 0;
};

struct Test2 {};

static size_t	find_call_count(const std::string & info) {
	size_t nCount = 1;
	std::string::size_type p = 0;
	while ((p = info.find(" <- ", p)) != std::string::npos) {
		++nCount;
		p = p + 2;
	}
	return nCount;
}

static void test_call_stack()
{
	BEGIN_CALL_AUTOINFO;

	std::stringstream o;
	ara::thread_context::dump_all_thread_state(o);
	std::string str = o.str();

	REQUIRE(str.find("test_call_stack") != std::string::npos);

	{
		BEGIN_CALL("Test in sub function");
		std::stringstream o2;
		ara::thread_context::dump_all_thread_state(o2);
		std::string str2 = o2.str();

		REQUIRE(find_call_count(str2) == 2);
		REQUIRE(str2.find("Test in sub function") != std::string::npos);
		REQUIRE(str2.find("test_call_stack") != std::string::npos);

		{
			BEGIN_CALL("Test in sub function2", clock());

			std::stringstream o4;
			ara::thread_context::dump_all_thread_state(o4);
			std::string str4 = o4.str();

			REQUIRE(find_call_count(str4) == 3);
		}
	}

	std::stringstream o3;
	ara::thread_context::dump_all_thread_state(o3);
	std::string str3 = o3.str();
	REQUIRE(find_call_count(str3) == 1);
	REQUIRE(str3.find("Test in sub function") == std::string::npos);
	REQUIRE(str3.find("test_call_stack") != std::string::npos);
}

TEST_CASE("threadext", "[base]") {

	SECTION("thread context") {
		auto & context = ara::thread_context::get();
		auto n = context.next_sn();
		REQUIRE(n == static_cast<uint64_t>(0));
		n = context.next_sn();
		REQUIRE(n == static_cast<uint64_t>(1));

		context.delete_on_thread_exit(new TestAutoDel);
	}

	SECTION("thread local storage pointer") {
		ara::thread_local_data_ptr<TLocalData>		a1;
		if (a1.empty()) {
			a1.reset(new TLocalData());
		}

		REQUIRE(a1->get() == static_cast<int>(0));
		a1->set(2);
		REQUIRE(a1->get() == static_cast<int>(2));

		auto		th2 = ara::make_thread([]() {
			ara::thread_local_data_ptr<TLocalData>		a1;
			REQUIRE(a1.empty());
		});

		th2.join();

		{
			ara::thread_local_data_ptr<TLocalData>		a1;
			REQUIRE(!a1.empty());
			REQUIRE(a1->get() == static_cast<int>(2));
		}
	}

	SECTION("thread local storage") {

		TLocalData & a1 = ara::thread_local_data<TLocalData>();

		REQUIRE(a1.get() == static_cast<int>(2));
		a1.set(3);
		REQUIRE(a1.get() == static_cast<int>(3));

		TLocalData & a2 = ara::thread_local_data<TLocalData, Test2>();
		REQUIRE(a2.get() == static_cast<int>(0));
		a2.set(4);
		REQUIRE(a2.get() == static_cast<int>(4));
		REQUIRE(a1.get() == static_cast<int>(3));

		TLocalData & a3 = ara::thread_local_data<TLocalData>();
		REQUIRE(a3.get() == static_cast<int>(3));

		int nValInThread = -1;
		std::thread		t([&nValInThread]() {
			TLocalData & a1 = ara::thread_local_data<TLocalData>();
			nValInThread = a1.get();
			ara::thread_context::destroy_context();
		});
		t.join();
		REQUIRE(nValInThread == static_cast<int>(0));

		auto t2 = ara::make_thread([&nValInThread]() {
			TLocalData & a1 = ara::thread_local_data<TLocalData>();
			a1.set(1000);
			nValInThread = a1.get();
			ara::thread_context::destroy_context();
		});
		auto t3 = ara::make_thread([]() {
			TLocalData & a1 = ara::thread_local_data<TLocalData>();
			a1.set(2000);
		});
		t2.join();
		t3.join();
		REQUIRE(nValInThread == static_cast<int>(1000));
	}

	SECTION("thread call stack") {
		test_call_stack();
	}
}
